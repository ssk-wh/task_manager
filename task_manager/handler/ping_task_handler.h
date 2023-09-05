#ifndef PING_TASK_HANDLER_H
#define PING_TASK_HANDLER_H

#include "abstract_task_handler.h"

/**
 * @brief The PingTaskHandler class
 * @note  服务端可以下发Ping类型的任务，用于检测客户端是否还存活
 */
class PingTaskHandler : public AbstractTaskHandler
{
    Q_OBJECT

public:
    explicit PingTaskHandler(const Task &info, QObject *parent = nullptr);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    Task m_info;
};

DECLARE_CREATE(PingTaskHandler);

#endif // PING_TASK_HANDLER_H
