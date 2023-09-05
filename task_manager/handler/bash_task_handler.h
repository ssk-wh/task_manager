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
    explicit BashTaskHandler(const Task &info, QObject *parent = nullptr);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    Task m_info;
};

DECLARE_CREATE(BashTaskHandler);

#endif // BASH_TASK_HANDLER_H
