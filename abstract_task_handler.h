#ifndef TASK_HANDLER_H
#define TASK_HANDLER_H

#include "task_info.h"

#include <QObject>
#include <QRunnable>

class AbstractTaskHandler : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit AbstractTaskHandler(QObject *parent = nullptr) : QObject(parent) {}

Q_SIGNALS:
    void taskStatusChanged(int id, TaskStatus status);      // 任务的状态发生变化时发出
    void taskHandleFinished(int id);                        // 任务完全执行结束时发出
};

#endif // TASK_HANDLER_H
