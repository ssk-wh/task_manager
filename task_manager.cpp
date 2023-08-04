#include "task_manager.h"
#include "task_dispatch.h"
#include "task_cache.h"

#include <QDebug>
#include <QTimer>
#include <QLoggingCategory>
#include <QThreadPool>
#include <QDBusMetaType>

#ifdef QT_DEBUG
#include <QRandomGenerator>
#endif

Q_LOGGING_CATEGORY(taskManager, "taskmanager")

// 从远程接收到任务，任务id必须是独一无二的，不能重复，这是一切的前提
TaskManager::TaskManager(QObject *parent)
    : QObject(parent)
    , m_displatcher(new TaskDispatcher(this))
    , m_taskCache(new TaskCache(this))
    , m_uploadResultTimer(new QTimer(this))
{
    qRegisterMetaType<TaskStatus>("TaskStatus");
    qRegisterMetaType<TaskInfo>("TaskInfo");
    qDBusRegisterMetaType<TaskInfo>();

    connect(m_uploadResultTimer, &QTimer::timeout, this, &TaskManager::onUploadResult);

    // 任务状态反馈 | 本地状态实时记录，防止异常
    connect(m_displatcher, &TaskDispatcher::taskStatusChanged, this, &TaskManager::onTaskStatusChanged);
    connect(m_displatcher, &TaskDispatcher::taskHandleFinished, this, &TaskManager::onTaskHandleFinished);

    // 从缓存查询历史任务，针对未执行完毕的任务，需要重新执行，执行完毕的任务，需要向服务器报告任务状态
    QMetaObject::invokeMethod(this, [ = ] {
        qCDebug(taskManager) << "Load history task";
        auto historyTaskMap = m_taskCache->loadFromCache();
        for (auto it = historyTaskMap.begin(); it != historyTaskMap.end(); ++it) {
            const TaskInfo &info = it.key();
            const TaskStatus &status = it.value();
            if (status == TaskStatus::Failure || status == TaskStatus::Success) {
                // 从服务器检查任务状态，如果本地是完毕状态(Success或Failure都是完毕状态)，服务端是未完毕，可能是本地执行完毕后未及时通知服务器，需要将状态同步到服务器
                TaskStatus serverStatus = taskStatusFromServer(info.task_id);
                if (serverStatus != status) {
                    m_finishedTaskMap.insert(info.task_id, status);
                    uploadTaskStatus(info.task_id, status);
                } else {
                    // 和服务器状态一致，可能是上传状态到返回器时，还没收到服务器的响应时正好本地进程退出了，这种不用处理
                }
            } else {
                // 取出任务继续执行
                qCDebug(taskManager) << "Rerun history task, id:" << info.task_id;
                dispatchTask(info);
            }
        }
    }, Qt::QueuedConnection);

    // 状态上传计时器,固定30s启动触发一次，每次都把完成的任务上传一遍，上传成功的任务会从待上传队列中删除
    m_uploadResultTimer->setInterval(30 *1000);
    m_uploadResultTimer->start();

#ifdef QT_DEBUG
    // 模拟启动后，在1分钟下发100条任务任务，涵盖所有任务类型，包含耗时、非耗时类型
    QTimer::singleShot(0, this, [ = ] {
        // 模拟收到新任务
        TaskInfo info;

        // 无效任务分发
        info.task_type = TaskType::Unknown;
        info.task_id = 1000;
        info.task_command = "bash -c \"sleep 3\"";
        dispatchTask(info);

        // Bash任务分发 (非耗时)
        info.task_type = TaskType::Bash;
        info.task_id ++;
        info.task_command = "bash -c \"ls -al\"";
        dispatchTask(info);

        for (int i = 0; i < 10; ++i) {
            quint32 value = QRandomGenerator::global()->generate();
            // Bash任务分发 (耗时)
            info.task_type = TaskType::Bash;
            info.task_id ++;
            info.task_command = QString("bash -c \"sleep %1\"").arg(value % (i + 1));
            dispatchTask(info);
        }
    });
#endif
}

TaskManager::~TaskManager()
{
    onUploadResult();
}

int TaskManager::taskCount()
{
    return QThreadPool::globalInstance()->activeThreadCount();
}

/**
 * @brief TaskManager::dispatchTask
 * @param info 任务内容
 * @note  从服务器获取的任务可以通过此接口往下分发
 */
void TaskManager::dispatchTask(const TaskInfo &info)
{
    qCDebug(taskManager) << "New task received, id:" << info.task_id;

    m_startTimeMap.insert(info.task_id, QDateTime::currentDateTime());
    m_taskCache->add(info);
    m_displatcher->dispatch(info);
}

void TaskManager::onTaskStatusChanged(int taskId, const TaskStatus &status)
{
    static QMap<TaskStatus, QString> map = {{TaskStatus::Ready, "Ready"}
                                            , {TaskStatus::Failure, "Failure"}
                                            , {TaskStatus::Success, "Success"}
                                            , {TaskStatus::InProgress, "InProgress"}};

    qCDebug(taskManager) << "Task status changed, id:" << taskId << ", status:" << map[status];
    m_taskCache->update(taskId, status);

    // 执行完毕的任务，记录下来，等待上报状态
    if (status == TaskStatus::Failure || status == TaskStatus::Success) {
        m_finishedTaskMap.insert(taskId, status);
    }
}

void TaskManager::onTaskHandleFinished(int taskId)
{
    qCDebug(taskManager) << "Task completed, id:" << taskId << ", elapsed:" << m_startTimeMap[taskId].secsTo(QDateTime::currentDateTime()) << "s";

    uploadTaskStatus(taskId, m_finishedTaskMap[taskId]);
}

// 上传任务执行情况
void TaskManager::onUploadResult()
{
//    qCDebug(taskManager) << "Upload all finished task status to server";
    for (auto it = m_finishedTaskMap.begin(); it != m_finishedTaskMap.end(); ++it) {
        uploadTaskStatus(it.key(), it.value());
    }
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
    return TaskStatus::Success;
}

void TaskManager::uploadTaskStatus(int taskId, const TaskStatus &status)
{
//    qCDebug(taskManager) << "Upload task status, id:" << taskId;

}
