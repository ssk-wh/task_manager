#ifndef MANAGER_H
#define MANAGER_H

#include <QObject>
#include <QMap>
#include <QDateTime>
#include <QPointer>
#include <QDBusContext>

#include "abstract_task_handler.h"

class TaskDispatcher;
class TaskCache;
class TaskReporter;
class QTimer;
/**
 * @brief The TaskManager class
 * The manager of tasks. It receives tasks remotely, delegates them to the dispatcher, caches task status via the cache, and notifies the remote side of execution results after completion.
 */
class TaskManager : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.github.ssk.TaskManager")

public:
    explicit TaskManager(QObject *parent = nullptr);
    ~ TaskManager();

Q_SIGNALS:
    void taskAboutToDispatch(const Task &task, const TaskStatus &status);
    void taskStatusChanged(int id, int status);
    void taskHandleFinished(int id);

public Q_SLOTS:
    void dispatchTask(int type, int id, const QString &name, const QString &command);

private Q_SLOTS:
    void onTaskStatusChanged(int taskId, const TaskStatus &status);
    void onTaskHandleFinished(int taskId);

    void onUploadSuccess(int taskId);
    void onUploadFailed(int taskId);
    void onUploadFinished();

    void tryUploadResult();

private:
    void registerMetaType();
    void initConnection();
    void loadHistoryTask();

    void dispatchTask(const Task &info);

    TaskStatus taskStatusFromServer(int taskId);

private:
    TaskDispatcher *m_displatcher;
    TaskCache *m_taskCache;
    QPointer<TaskReporter> m_taskReporter;

    // As long as there are completed tasks, periodically trigger task status upload
    QTimer *m_periodUploadTimer;

    QMap<int, QDateTime> m_startTimeMap;
    QMap<int, TaskStatus> m_finishedTaskMap;
};

#endif // MANAGER_H
