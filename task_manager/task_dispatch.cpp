#include "task_dispatch.h"
#include "bash_task_handler.h"
#include "settings_task_handler.h"
#include "strategy_task_handler.h"
#include "ping_task_handler.h"

#include <QDebug>
#include <QLoggingCategory>
#include <QThreadPool>

Q_LOGGING_CATEGORY(taskdispatcher, "dispatcher")

TaskDispatcher::TaskDispatcher(QObject *parent)
    : QObject(parent)
{
    m_handlerMap.insert(TaskType::Ping,       reinterpret_cast<AbstractTaskHandler* (*)(const Task&, QObject*)>(&CREATE_PTR(PingTaskHandler)));
    m_handlerMap.insert(TaskType::Bash,       reinterpret_cast<AbstractTaskHandler* (*)(const Task&, QObject*)>(&CREATE_PTR(BashTaskHandler)));
    m_handlerMap.insert(TaskType::Settings,   reinterpret_cast<AbstractTaskHandler* (*)(const Task&, QObject*)>(&CREATE_PTR(SettingsTaskHandler)));
    m_handlerMap.insert(TaskType::Strategy,   reinterpret_cast<AbstractTaskHandler* (*)(const Task&, QObject*)>(&CREATE_PTR(StrategyTaskHandler)));
}

void TaskDispatcher::dispatch(const Task &info)
{
    TaskHandlerConstructor handlerConstructor = m_handlerMap.value(info.type, nullptr);
    if (handlerConstructor) {
        AbstractTaskHandler *handler = handlerConstructor(info, this);
        Q_ASSERT_X(handler, "Task dispatch error", "All handler classes must implement the 'create' entry function"
                                                   ", and you can use the DECLARE_CREATE macro for a quick implementation.");
        connect(handler, &AbstractTaskHandler::taskStatusChanged, this, &TaskDispatcher::taskStatusChanged);
        connect(handler, &AbstractTaskHandler::taskHandleFinished, this, &TaskDispatcher::taskHandleFinished);

        handler->setAutoDelete(true);

        // 策略型任务优先执行，这些一般会明显改变用户电脑上一些显示外观方面的配置，需要优先执行从而让用户尽快感知
        int ptiority = (info.type == TaskType::Strategy) ? QThread::HighPriority : QThread::NormalPriority;
        QThreadPool::globalInstance()->start(handler, ptiority);

        // 记录剩余未执行完毕的任务
        m_dispatchTaskMap.insert(handler, info.id);
        connect(handler, &QObject::destroyed, this, [ = ] {
            qCDebug(taskdispatcher) << "Task handler destroyed, id:" << m_dispatchTaskMap.value(handler);
            m_dispatchTaskMap.remove(handler);
            Q_EMIT handlerCountChanged(m_dispatchTaskMap.size());
        });

    } else {
        Q_EMIT taskStatusChanged(info.id, TaskStatus::Ready);
        qCWarning(taskdispatcher) << "Unimplement task type:" << static_cast<int>(info.type)
                                  << ", id:" << info.id
                                  << ", name:" << info.name;
        Q_EMIT taskStatusChanged(info.id, TaskStatus::Failure);
        Q_EMIT taskHandleFinished(info.id);
    }
}

int TaskDispatcher::handlerCount()
{
    return m_dispatchTaskMap.size();
}
