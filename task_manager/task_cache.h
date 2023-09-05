#ifndef TASK_CACHE_H
#define TASK_CACHE_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QDateTime>

#include "task.h"

/**
 * @brief The TaskCache class
 * 用于保存服务任务状态，数据存储在本地。即使进程异常关闭，下次启动也能从本地获取任务的执行状态，
 * 从而判断是继续执行还是上报完成状态。避免进程异常退出导致任务丢失。
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
