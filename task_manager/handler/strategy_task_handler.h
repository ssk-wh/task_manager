#ifndef STRATEGY_TASK_HANDLER_H
#define STRATEGY_TASK_HANDLER_H

#include "abstract_task_handler.h"

/**
 * @brief The StrategyTaskHandler class
 * @note  统一策略下发，负责一系列策略的拉取并执行
 * @note  适用场景：服务端配置了一系列设置(设置项总是固定的)，要求客户端立即全部拉取到本地并返回设置情况
 */
class StrategyTaskHandler : public AbstractTaskHandler
{
    Q_OBJECT

public:
    explicit StrategyTaskHandler(const Task &info, QObject *parent = nullptr);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    Task m_info;
};

DECLARE_CREATE(StrategyTaskHandler);

#endif // STRATEGY_TASK_HANDLER_H
