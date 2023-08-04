#include "task.h"

QDBusArgument &operator<<(QDBusArgument &argument, const Task &info)
{
    argument.beginStructure();
    argument << static_cast<int>(info.type) << info.id << info.name << info.command;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Task &info)
{
    argument.beginStructure();
    int type = -1;
    argument >> type >> info.id >> info.name >> info.command;
    info.type = static_cast<TaskType>(type);
    argument.endStructure();
    return argument;
}

bool operator<(const Task &a, const Task &other)
{
    return a.id < other.id;
}
