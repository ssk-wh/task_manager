#ifndef BASH_TASK_HANDLER_H
#define BASH_TASK_HANDLER_H

#include "abstract_task_handler.h"

/**
 * @brief The BashTaskHandler class
 * 任务的执行者，此类只针对Bash脚本类型的任务，放到线程中单独运行
 */
class BashTaskHandler : public AbstractTaskHandler
{
    Q_OBJECT
public:
    explicit BashTaskHandler(const TaskInfo &info, QObject *parent = nullptr);

protected:
    virtual void run();

private:
    TaskInfo m_info;
};

#endif // BASH_TASK_HANDLER_H
