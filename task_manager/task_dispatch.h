#ifndef TASK_DISPATCH_H
#define TASK_DISPATCH_H

#include <QObject>
#include <QMap>

#include "task.h"

class AbstractTaskHandler;
/**
 * @brief The TaskDispatcher class
 * 仅负责任务的分发和任务执行状态的转发
 */
class TaskDispatcher : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int handlerCount READ handlerCount NOTIFY handlerCountChanged)

public:
    explicit TaskDispatcher(QObject *parent = nullptr);

    void dispatch(const Task &info);

    int handlerCount();

Q_SIGNALS:
    void taskStatusChanged(int id, TaskStatus status);
    void taskHandleFinished(int id);
    void handlerCountChanged(int count);

private:
    typedef AbstractTaskHandler *(*TaskHandlerConstructor)(const Task&, QObject*);
    QMap<TaskType, TaskHandlerConstructor> m_handlerMap;

    QMap<AbstractTaskHandler *, int> m_dispatchTaskMap;
};

#endif // TASK_DISPATCH_H
