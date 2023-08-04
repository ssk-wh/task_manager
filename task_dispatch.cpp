#include "task_dispatch.h"
#include "bash_task_handler.h"

#include <QDebug>
#include <QLoggingCategory>
#include <QThreadPool>

Q_LOGGING_CATEGORY(taskdispatcher, "taskdispatcher")

TaskDispatcher::TaskDispatcher(QObject *parent)
    : QObject(parent)
{

}

void TaskDispatcher::dispatch(const TaskInfo &info)
{
    AbstractTaskHandler *handler = nullptr;
    switch (info.task_type) {
    case TaskType::Bash:
        handler = new BashTaskHandler(info, this);
        break;
    case TaskType::Unknown:
    case TaskType::Lua:
    case TaskType::Python:
    case TaskType::Crontab:
    case TaskType::NoCallBack:
        qCWarning(taskdispatcher) << "Unknown task type:" << static_cast<int>(info.task_type) << ", id:" << info.task_id;
        Q_EMIT taskStatusChanged(info.task_id, TaskStatus::Failure);
        Q_EMIT taskHandleFinished(info.task_id);
        return;
    }

    Q_ASSERT(handler);

    connect(handler, &AbstractTaskHandler::taskStatusChanged, this, &TaskDispatcher::taskStatusChanged);
    connect(handler, &AbstractTaskHandler::taskHandleFinished, this, &TaskDispatcher::taskHandleFinished);

    QThreadPool::globalInstance()->start(handler);

    // TODO 监控执行超时的线程，默认30s
}
