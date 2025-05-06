#include "dispatcher.h"
#include "bash_task_handler.h"
#include "settings_task_handler.h"
#include "strategy_task_handler.h"
#include "ping_task_handler.h"

#include <QDebug>
#include <QLoggingCategory>
#include <QThreadPool>

Q_LOGGING_CATEGORY(taskdispatcher, "dispatcher")

Dispatcher::Dispatcher(QObject *parent)
    : QObject(parent)
{
    m_handlerMap.insert(TaskType::Ping,       reinterpret_cast<AbstractTaskHandler* (*)(const Task&, QObject*)>(&GET_HANDLER_PTR(PingTaskHandler)));
    m_handlerMap.insert(TaskType::Bash,       reinterpret_cast<AbstractTaskHandler* (*)(const Task&, QObject*)>(&GET_HANDLER_PTR(BashTaskHandler)));
    m_handlerMap.insert(TaskType::Settings,   reinterpret_cast<AbstractTaskHandler* (*)(const Task&, QObject*)>(&GET_HANDLER_PTR(SettingsTaskHandler)));
    m_handlerMap.insert(TaskType::Strategy,   reinterpret_cast<AbstractTaskHandler* (*)(const Task&, QObject*)>(&GET_HANDLER_PTR(StrategyTaskHandler)));
}

void Dispatcher::dispatch(const Task &info)
{
    TaskHandlerConstructor handlerConstructor = m_handlerMap.value(info.type, nullptr);
    if (handlerConstructor) {
        AbstractTaskHandler *handler = handlerConstructor(info, this);
        Q_ASSERT_X(handler, "Task dispatch error", "All handler classes must implement the 'create' entry function"
                                                   ", and you can use the DECLARE_CREATE macro for a quick implementation.");
        connect(handler, &AbstractTaskHandler::taskStatusChanged, this, &Dispatcher::taskStatusChanged);
        connect(handler, &AbstractTaskHandler::taskHandleFinished, this, &Dispatcher::taskHandleFinished);

        handler->setAutoDelete(true);

        // Strategy tasks have higher priority as they often change user-visible settings
        int priority = (info.type == TaskType::Strategy) ? QThread::HighPriority : QThread::NormalPriority;
        QThreadPool::globalInstance()->start(handler, priority);

        // Track unfinished tasks
        m_dispatchTaskMap.insert(handler, info.id);
        connect(handler, &QObject::destroyed, this, [=] {
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

int Dispatcher::handlerCount()
{
    return m_dispatchTaskMap.size();
}
