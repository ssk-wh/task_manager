#include "ping_task_handler.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pingTaskHandler, "handler-ping")

PingTaskHandler::PingTaskHandler(const Task &info, QObject *parent)
    : AbstractTaskHandler(parent)
    , m_info(info)
{
    Q_ASSERT(m_info.type == TaskType::Ping);
}

void PingTaskHandler::run()
{
    Q_EMIT taskStatusChanged(m_info.id, TaskStatus::Ready);

    // just send live message to server
    qCDebug(pingTaskHandler) << "Handle ping, id:" << m_info.id;

    Q_EMIT taskStatusChanged(m_info.id, TaskStatus::Success);

    Q_EMIT taskHandleFinished(m_info.id);
}
