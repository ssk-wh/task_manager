#include "task_manager.h"
#include "task_dispatch.h"
#include "task_cache.h"
#include "task_reporter.h"

#include <QDebug>
#include <QTimer>
#include <QLoggingCategory>
#include <QThreadPool>
#include <QDBusMetaType>

Q_LOGGING_CATEGORY(taskManager, "taskmanager")

// 从远程接收到任务，任务id必须是独一无二的，不能重复，这是一切的前提
TaskManager::TaskManager(QObject *parent)
    : QObject(parent)
    , m_displatcher(new TaskDispatcher(this))
    , m_taskCache(new TaskCache(this))
    , m_periodUploadTimer(new QTimer(this))
{
    registerMetaType();
    initConnection();

#ifdef ENABLE_LOAD_HISTORY_TASK
    loadHistoryTask();
#endif

    m_periodUploadTimer->setInterval(10 * 1000);
}

TaskManager::~TaskManager()
{
    auto pool = QThreadPool::globalInstance();
    if (pool->activeThreadCount() > 0) {
        qCDebug(taskManager) << "Wait for all tasks finished";
        pool->waitForDone();
    }
}

void TaskManager::dispatchTask(int type, int id, const QString &name, const QString &command)
{   
    Task info {static_cast<TaskType>(type), id, name, command};
    dispatchTask(info);
}

/**
 * @brief TaskManager::dispatchTask
 * @param info 任务内容
 * @note  从服务器获取的任务可以通过此接口往下分发
 */
void TaskManager::dispatchTask(const Task &info)
{
    static QMap<TaskType, QString> map = {{TaskType::Unknown, "Unknown"}
                                          , {TaskType::Ping, "Ping"}
                                          , {TaskType::Bash, "Bash"}
                                          , {TaskType::Settings, "Settings"}
                                          , {TaskType::Strategy, "Strategy"}
                                          , {TaskType::MaximumType, "MaximumType"}};

    qCDebug(taskManager) << "New task received, id:" << info.id << ", type:" << map[info.type];

    Q_EMIT taskAboutToDispatch(info, TaskStatus::Received);

    m_displatcher->dispatch(info);
}

void TaskManager::onTaskStatusChanged(int taskId, const TaskStatus &status)
{
    Q_EMIT taskStatusChanged(taskId, static_cast<int>(status));

    // 记录任务开始时的时间
    if (status == TaskStatus::Ready) {
        m_startTimeMap.insert(taskId, QDateTime::currentDateTime());
    }

    // 执行完毕的任务，记录下来，等待上报状态
    if (status == TaskStatus::Failure || status == TaskStatus::Success) {
        m_finishedTaskMap.insert(taskId, status);
    }
}

void TaskManager::onTaskHandleFinished(int taskId)
{
    Q_EMIT taskHandleFinished(taskId);

    auto startTime = m_startTimeMap[taskId];
    if (startTime.isValid()) {
        qint64 elapsed = startTime.secsTo(QDateTime::currentDateTime());
        qCDebug(taskManager) << "Task completed, id:" << taskId << ", elapsed:" << elapsed << "s";
    } else {
        qCDebug(taskManager) << "Task completed, id:" << taskId;
    }

    m_startTimeMap.remove(taskId);

    Q_ASSERT_X(m_finishedTaskMap.contains(taskId), qPrintable(QString("Task(%1) status").arg(taskId))
               , "Error from internal, task need to send `Success/Failure status before task finished");

    if (!m_periodUploadTimer->isActive()) {
        m_periodUploadTimer->start();
    }
}

void TaskManager::tryUploadResult()
{
    qCDebug(taskManager) << "Remaining check, handling task count:" << m_displatcher->handlerCount() << ", finished task count:" << m_finishedTaskMap.size();

    if (m_finishedTaskMap.isEmpty()) {
        qCDebug(taskManager) << "All task upload finished";
        return;
    }

    // 如果之前的线程的工作做完了，会被自动删除，这里判断删除了就重新起一个新的对象来继续上传其他数据
    if (m_taskReporter.isNull()) {
        m_taskReporter = new TaskReporter(m_finishedTaskMap, this);
        m_taskReporter.data()->setAutoDelete(true);

        connect(m_taskReporter.data(), &TaskReporter::uploadSuccess, this, &TaskManager::onUploadSuccess);
        connect(m_taskReporter.data(), &TaskReporter::uploadFailed, this, &TaskManager::onUploadFailed);
        connect(m_taskReporter.data(), &TaskReporter::uploadFinished, this, &TaskManager::onUploadFinished);
        connect(m_taskReporter.data(), &TaskReporter::uploadSuccess, m_taskCache, &TaskCache::remove);

        qCDebug(taskManager) << "Start upload task status, task count:" << m_finishedTaskMap.size();

        QThreadPool::globalInstance()->start(m_taskReporter.data(), QThread::HighPriority);
    }
}

void TaskManager::onUploadSuccess(int taskId)
{
    qCDebug(taskManager).noquote() << QString("Upload status success, id: %1").arg(taskId);
    m_finishedTaskMap.remove(taskId);

    if (m_finishedTaskMap.size() <= 0) {
        m_periodUploadTimer->stop();
    }
}

void TaskManager::onUploadFailed(int taskId)
{
    qCDebug(taskManager).noquote() << QString("Failed to upload status, id: %1, will be retry after a while...").arg(taskId);
}

void TaskManager::onUploadFinished()
{
    qCDebug(taskManager) << "Current loop upload finished";
}

void TaskManager::registerMetaType()
{
    qRegisterMetaType<TaskStatus>("TaskStatus");
    qRegisterMetaType<Task>("TaskInfo");
    qDBusRegisterMetaType<Task>();
}

void TaskManager::initConnection()
{
    connect(this, &TaskManager::taskAboutToDispatch, m_taskCache, &TaskCache::add);

    // 任务状态反馈 | 本地状态实时记录，防止异常
    connect(m_displatcher, &TaskDispatcher::taskStatusChanged, this, &TaskManager::onTaskStatusChanged);
    connect(m_displatcher, &TaskDispatcher::taskStatusChanged, m_taskCache, &TaskCache::update);
    connect(m_displatcher, &TaskDispatcher::taskHandleFinished, this, &TaskManager::onTaskHandleFinished);

    connect(m_periodUploadTimer, &QTimer::timeout, this, &TaskManager::tryUploadResult);
}

void TaskManager::loadHistoryTask()
{
    QMetaObject::invokeMethod(this, [ = ] {
        qCDebug(taskManager) << "Load history task start";
        auto historyTaskMap = m_taskCache->loadFromCache();
        for (auto it = historyTaskMap.begin(); it != historyTaskMap.end(); ++it) {
            const Task &info = it.key();
            const TaskStatus &status = it.value();
            if (status == TaskStatus::Failure || status == TaskStatus::Success) {
                // 从服务器检查任务状态，如果本地是完毕状态(Success或Failure都是完毕状态)，服务端是未完毕，可能是本地执行完毕后未及时通知服务器，需要将状态同步到服务器
                TaskStatus serverStatus = taskStatusFromServer(info.id);
                if (serverStatus != status) {
                    m_finishedTaskMap.insert(info.id, status);
                    qCDebug(taskManager) << "Task already inished, id:" << info.id << ", wait for upload";
                    if (!m_periodUploadTimer->isActive()) {
                        m_periodUploadTimer->start();
                    }
                } else {
                    // 和服务器状态一致，可能是上传状态到返回器时，还没收到服务器的响应时正好本地进程退出了，这种不用处理
                }
            } else {
                // 取出任务继续执行
                qCDebug(taskManager) << "Rerun history task, id:" << info.id;
                dispatchTask(info);
            }
        }
        qCDebug(taskManager) << "Load history task end";
    }, Qt::QueuedConnection);
}

/**
 * @brief TaskManager::taskStatusFromServer
 * @param taskId
 * @return  服务器应该提供查询接口，可以得知任务的执行状态
 * @note  不过我们这里只是一个验证型demo，所以没有实现
 */
TaskStatus TaskManager::taskStatusFromServer(int taskId)
{
    Q_UNUSED(taskId);
    // TODO
    return TaskStatus::Ready;
}
