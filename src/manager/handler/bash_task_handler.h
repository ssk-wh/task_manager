#ifndef BASH_TASK_HANDLER_H
#define BASH_TASK_HANDLER_H

#include "abstract_task_handler.h"

/**
 * @brief The BashTaskHandler class
 * The executor for tasks. This class is specifically for tasks of Bash script type and is run separately in a thread.
 */
class BashTaskHandler : public AbstractTaskHandler
{
    Q_OBJECT

public:
    explicit BashTaskHandler(const Task &info, QObject *parent = nullptr);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    Task m_info;
};

DECLARE_CREATE(BashTaskHandler);

#endif // BASH_TASK_HANDLER_H
