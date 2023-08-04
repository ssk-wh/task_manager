#include "task_info.h"

QDBusArgument &operator<<(QDBusArgument &argument, const TaskInfo &info)
{
    argument.beginStructure();
    argument << static_cast<int>(info.task_type) << info.task_id << info.task_command;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, TaskInfo &info)
{
    argument.beginStructure();
    int type = -1;
    argument >> type >> info.task_id >> info.task_command;
    info.task_type = static_cast<TaskType>(type);
    argument.endStructure();
    return argument;
}

bool operator<(const TaskInfo &a, const TaskInfo &other)
{
    return a.task_id < other.task_id;
}
