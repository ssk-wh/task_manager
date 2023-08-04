#ifndef MANAGER_H
#define MANAGER_H

#include <QObject>
#include <QMap>
#include <QDateTime>

#include "abstract_task_handler.h"

class TaskDispatcher;
class TaskCache;
class QTimer;
/**
 * @brief The TaskManager class
 * 任务的管理者，从远程接收任务，交由分发器进行分发，通过缓存器缓存任务状态，执行完毕后通知远程执行情况
 */
class TaskManager : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.github.ssk-wh.TaskManager")
    Q_PROPERTY(int taskCount READ taskCount)

public:
    explicit TaskManager(QObject *parent = nullptr);
    ~TaskManager();

    int taskCount();

public Q_SLOTS:
    void dispatchTask(const TaskInfo &info);

private Q_SLOTS:
    void onTaskStatusChanged(int taskId, const TaskStatus &status);
    void onTaskHandleFinished(int taskId);
    void onUploadResult();

private:
    TaskStatus taskStatusFromServer(int taskId);
    void uploadTaskStatus(int taskId, const TaskStatus &status);

private:
    TaskDispatcher *m_displatcher;
    TaskCache *m_taskCache;

    QTimer *m_uploadResultTimer;

    QMap<int, QDateTime> m_startTimeMap;
    QMap<int, TaskStatus> m_finishedTaskMap;
};

#endif // MANAGER_H
