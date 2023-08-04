#ifndef TASK_DISPATCH_H
#define TASK_DISPATCH_H
#include <QObject>
#include <QMap>

#include "abstract_task_handler.h"

/**
 * @brief The TaskDispatcher class
 * 负责任务的分发，以及任务执行状态的转发
 */
class TaskDispatcher : public QObject
{
    Q_OBJECT
public:
    explicit TaskDispatcher(QObject *parent = nullptr);

    void dispatch(const TaskInfo &info);

Q_SIGNALS:
    void taskStatusChanged(int id, TaskStatus status);
    void taskHandleFinished(int id);
};

#endif // TASK_DISPATCH_H
