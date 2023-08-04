#ifndef TASK_INFO_H
#define TASK_INFO_H

#include <QString>
#include <QDBusMetaType>

enum TaskType {
    Unknown,
    Bash,
    Lua,
    Python,
    NoCallBack,
    Crontab
};

enum TaskStatus {
    Ready,          // 刚收到任务，准备执行
    InProgress,     // 执行中
    Success,        // 执行成功
    Failure         // 执行失败
};

struct TaskInfo {
    TaskType task_type = TaskType::Unknown;
    int task_id = -1;
    QString task_command;
};
Q_DECLARE_METATYPE(TaskInfo)

bool operator<(const TaskInfo& a, const TaskInfo &other);
QDBusArgument &operator<<(QDBusArgument&, const TaskInfo&);
const QDBusArgument &operator>>(const QDBusArgument&, TaskInfo&);

#endif // TASK_INFO_H
