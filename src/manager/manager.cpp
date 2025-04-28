#include "manager.h"
#include "dispatcher.h"
#include "cache.h"
#include "reporter.h"

#include <QDebug>
#include <QTimer>
#include <QLoggingCategory>
#include <QThreadPool>
#include <QDBusMetaType>

Q_LOGGING_CATEGORY(taskManager, "taskmanager")

// When receiving a task from remote, the task id must be unique and not duplicated. This is a fundamental requirement.
Manager::Manager(QObject *parent)
    : QObject(parent)
    , m_displatcher(new Dispatcher(this))
    , m_taskCache(new Cache(this))
    , m_periodUploadTimer(new QTimer(this))
{
    registerMetaType();
    initConnection();

#ifdef ENABLE_LOAD_HISTORY_TASK
    loadHistoryTask();
#endif

    m_periodUploadTimer->setInterval(10 * 1000);
}

Manager::~Manager()
{
    auto pool = QThreadPool::globalInstance();
    if (pool->activeThreadCount() > 0) {
        qCDebug(taskManager) << "Wait for all tasks finished";
        pool->waitForDone();
    }
}

void Manager::dispatchTask(int type, int id, const QString &name, const QString &command)
{   
    Task info {static_cast<TaskType>(type), id, name, command};
    dispatchTask(info);
}

/**
 * @brief Manager::dispatchTask
 * @param info Task information
 * @note  Tasks received from the server can be dispatched through this interface
 */
void Manager::dispatchTask(const Task &info)
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

void Manager::onTaskStatusChanged(int taskId, const TaskStatus &status)
{
    Q_EMIT taskStatusChanged(taskId, static_cast<int>(status));

    // Record the start time when the task status is Ready
    if (status == TaskStatus::Ready) {
        m_startTimeMap.insert(taskId, QDateTime::currentDateTime());
    }

    // For tasks that are finished (Success or Failure), record them for later status upload
    if (status == TaskStatus::Failure || status == TaskStatus::Success) {
        m_finishedTaskMap.insert(taskId, status);
    }
}

void Manager::onTaskHandleFinished(int taskId)
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

void Manager::tryUploadResult()
{
    qCDebug(taskManager) << "Remaining check, handling task count:" << m_displatcher->handlerCount() << ", finished task count:" << m_finishedTaskMap.size();

    if (m_finishedTaskMap.isEmpty()) {
        qCDebug(taskManager) << "All task upload finished";
        return;
    }

    // If the previous thread's work is done and the object is deleted, create a new object to continue uploading other data
    if (m_taskReporter.isNull()) {
        m_taskReporter = new Reporter(m_finishedTaskMap, this);
        m_taskReporter.data()->setAutoDelete(true);

        connect(m_taskReporter.data(), &Reporter::uploadSuccess, this, &Manager::onUploadSuccess);
        connect(m_taskReporter.data(), &Reporter::uploadFailed, this, &Manager::onUploadFailed);
        connect(m_taskReporter.data(), &Reporter::uploadFinished, this, &Manager::onUploadFinished);
        connect(m_taskReporter.data(), &Reporter::uploadSuccess, m_taskCache, &Cache::remove);

        qCDebug(taskManager) << "Start upload task status, task count:" << m_finishedTaskMap.size();

        QThreadPool::globalInstance()->start(m_taskReporter.data(), QThread::HighPriority);
    }
}

void Manager::onUploadSuccess(int taskId)
{
    qCDebug(taskManager).noquote() << QString("Upload status success, id: %1").arg(taskId);
    m_finishedTaskMap.remove(taskId);

    if (m_finishedTaskMap.size() <= 0) {
        m_periodUploadTimer->stop();
    }
}

void Manager::onUploadFailed(int taskId)
{
    qCDebug(taskManager).noquote() << QString("Failed to upload status, id: %1, will be retry after a while...").arg(taskId);
}

void Manager::onUploadFinished()
{
    qCDebug(taskManager) << "Current loop upload finished";
}

void Manager::registerMetaType()
{
    qRegisterMetaType<TaskStatus>("TaskStatus");
    qRegisterMetaType<Task>("TaskInfo");
    qDBusRegisterMetaType<Task>();
}

void Manager::initConnection()
{
    connect(this, &Manager::taskAboutToDispatch, m_taskCache, &Cache::add);
    // Task status feedback | Real-time local status recording to prevent exceptions
    connect(m_displatcher, &Dispatcher::taskStatusChanged, this, &Manager::onTaskStatusChanged);
    connect(m_displatcher, &Dispatcher::taskStatusChanged, m_taskCache, &Cache::update);
    connect(m_displatcher, &Dispatcher::taskHandleFinished, this, &Manager::onTaskHandleFinished);

    connect(m_periodUploadTimer, &QTimer::timeout, this, &Manager::tryUploadResult);
}

void Manager::loadHistoryTask()
{
    QMetaObject::invokeMethod(this, [ = ] {
        qCDebug(taskManager) << "Load history task start";
        auto historyTaskMap = m_taskCache->loadFromCache();
        for (auto it = historyTaskMap.begin(); it != historyTaskMap.end(); ++it) {
            const Task &info = it.key();
            const TaskStatus &status = it.value();
            if (status == TaskStatus::Failure || status == TaskStatus::Success) {
                // Check the task status from the server. If the local status is finished (Success or Failure), but the server status is not finished,
                // it may be that the local execution finished but failed to notify the server in time. Need to sync the status to the server.
                TaskStatus serverStatus = taskStatusFromServer(info.id);
                if (serverStatus != status) {
                    m_finishedTaskMap.insert(info.id, status);
                    qCDebug(taskManager) << "Task already finished, id:" << info.id << ", wait for upload";
                    if (!m_periodUploadTimer->isActive()) {
                        m_periodUploadTimer->start();
                    }
                } else {
                    // If the status is consistent with the server, it may be that the local process exited before receiving the server's response after uploading.
                    // No need to handle this case.
                }
            } else {
                // Resume unfinished tasks
                qCDebug(taskManager) << "Rerun history task, id:" << info.id;
                dispatchTask(info);
            }
        }
        qCDebug(taskManager) << "Load history task end";
    }, Qt::QueuedConnection);
}

/**
 * @brief Manager::taskStatusFromServer
 * @param taskId
 * @return The server should provide a query interface to obtain the execution status of the task.
 * @note  However, this is just a demo for validation purposes, so it is not implemented here.
 */
TaskStatus Manager::taskStatusFromServer(int taskId)
{
    Q_UNUSED(taskId);
    // TODO
    return TaskStatus::Ready;
}
