#ifndef TASK_CACHE_H
#define TASK_CACHE_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QDateTime>

#include "task.h"

/**
 * @brief The TaskCache class
 * Used to save the status of service tasks, with data stored locally. 
 * Even if the process is abnormally closed, the next startup can retrieve the execution status of tasks from local storage,
 * allowing the system to decide whether to continue execution or report completion status.
 * This prevents task loss caused by abnormal process termination.
 */
class TaskCache : public QObject
{
    Q_OBJECT
public:
    explicit TaskCache(QObject *parent = nullptr);

    QMap<Task, TaskStatus> loadFromCache() const;

#ifdef RECORD_ALL_TASK_INFO
    struct TaskDumpMessage{
        Task info;
        TaskStatus status;
        QDateTime createTime;
        QDateTime startTime;
        QDateTime endTime;
    };
#endif

public Q_SLOTS:
    void add(const Task &info, const TaskStatus &status = TaskStatus::Ready);
    void update(int taskId, const TaskStatus &status);
    void remove(int taskId);

private:
    void writeJsonToFile(const QString &fileName, const QJsonObject &obj) const;
    QJsonObject readJsonFromFile(const QString &fileName) const;

private:
    QString m_configPath;

#ifdef RECORD_ALL_TASK_INFO
    QList<TaskDumpMessage> m_taskList;
#endif
};

#endif // TASK_CACHE_H
