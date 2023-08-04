#ifndef STRATEGY_TASK_HANDLER_H
#define STRATEGY_TASK_HANDLER_H

#include "abstract_task_handler.h"

/**
 * @brief The StrategyTaskHandler class
 * @note  Unified strategy delivery, responsible for fetching and executing a series of strategies
 * @note  Applicable scenario: The server configures a series of settings (the items are always fixed), requiring the client to immediately fetch all settings locally and return their status
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
