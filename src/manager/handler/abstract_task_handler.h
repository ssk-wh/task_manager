#ifndef TASK_HANDLER_H
#define TASK_HANDLER_H

#include "task.h"

#include <QObject>
#include <QRunnable>

#define GET_HANDLER_PTR(Handler)  __##Handler##__create
#define DECLARE_CREATE(Handler)                    \
static Handler *__##Handler##__create(const Task &info, QObject *parent) { \
    return new Handler(info, parent);                       \
}
class AbstractTaskHandler : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit AbstractTaskHandler(QObject *parent = nullptr) : QObject(parent) {}

Q_SIGNALS:
    void taskStatusChanged(int id, const TaskStatus &status);       // Emitted when the task status changes
    void taskHandleFinished(int id);                                // Emitted when the task is completely finished
};

#endif // TASK_HANDLER_H
