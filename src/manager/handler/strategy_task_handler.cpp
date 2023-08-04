#include "strategy_task_handler.h"

StrategyTaskHandler::StrategyTaskHandler(const Task &info, QObject *parent)
    : AbstractTaskHandler(parent)
    , m_info(info)
{
    Q_ASSERT(m_info.type == TaskType::Strategy);
}

void StrategyTaskHandler::run()
{
    Q_EMIT taskStatusChanged(m_info.id, TaskStatus::Ready);

    // Get wallpaper settings and apply them
    // Get desktop module disable configuration and apply it
    // Get auto-start application configuration and apply it
    // Get other personalization configurations and apply them

    Q_EMIT taskStatusChanged(m_info.id, TaskStatus::Success);

    Q_EMIT taskHandleFinished(m_info.id);
}
