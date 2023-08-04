#ifndef TASK_CACHE_H
#define TASK_CACHE_H

#include <QObject>
#include <QList>

#include "abstract_task_handler.h"

/**
 * @brief The TaskCache class
 * 服务任务状态的保存，数据存储在本地
 * 即使进程异常关闭了，下次启动也能从本地得知任务的执行状态
 * 从而判断是继续执行还是上报完成状态，可以很好地避免进程异常退出任务丢失的情况
 */
class TaskCache : public QObject
{
public:
    explicit TaskCache(QObject *parent = nullptr);

    const QMap<TaskInfo, TaskStatus> &loadFromCache();

    void add(const TaskInfo &info, const TaskStatus &status = TaskStatus::Ready);
    void update(int taskId, const TaskStatus &status);
    void remove(int taskId);

private:
    QMap<TaskInfo, TaskStatus> m_historyTaskMap;
    QString m_configPath;
};

#endif // TASK_CACHE_H
