#ifndef PING_TASK_HANDLER_H
#define PING_TASK_HANDLER_H

#include "abstract_task_handler.h"

/**
 * @brief The PingTaskHandler class
 * @note  The server can issue Ping type tasks to check if the client is still alive.
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
