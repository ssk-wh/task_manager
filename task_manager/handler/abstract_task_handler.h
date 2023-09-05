#ifndef TASK_HANDLER_H
#define TASK_HANDLER_H

#include "task.h"

#include <QObject>
#include <QRunnable>

#define CREATE_PTR(Handler)  __##Handler##create
#define DECLARE_CREATE(Handler)                    \
static Handler *__##Handler##create(const Task &info, QObject *parent) { \
    return new Handler(info, parent);                       \
}
class AbstractTaskHandler : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit AbstractTaskHandler(QObject *parent = nullptr) : QObject(parent) {}

Q_SIGNALS:
    void taskStatusChanged(int id, const TaskStatus &status);       // 任务的状态发生变化时发出
    void taskHandleFinished(int id);                                // 任务完全执行结束时发出
};

#endif // TASK_HANDLER_H
