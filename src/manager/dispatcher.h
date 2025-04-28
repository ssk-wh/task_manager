#ifndef DISPATCH_H
#define DISPATCH_H

#include <QObject>
#include <QMap>

#include "task.h"

class AbstractTaskHandler;
/**
 * @brief The Dispatcher class
 * Responsible only for dispatching tasks and forwarding task execution status.
 */
class Dispatcher : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int handlerCount READ handlerCount NOTIFY handlerCountChanged)

public:
    explicit Dispatcher(QObject *parent = nullptr);

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

#endif // DISPATCH_H
