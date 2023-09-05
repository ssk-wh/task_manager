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

    // 获取壁纸设置并设置
    // 获取桌面模块禁用配置并设置
    // 获取自启应用配置并设置
    // 获取其他个性化配置并设置

    Q_EMIT taskStatusChanged(m_info.id, TaskStatus::Success);

    Q_EMIT taskHandleFinished(m_info.id);
}
