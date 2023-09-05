#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QDBusMetaType>

enum TaskType {
    Unknown = 0,
    Ping,               // 检查客户端是否存活，收到后应直接以约定的方式响应服务端
    Bash,               // 命令的内容一般是bash语句
    Settings,           // 命令的内容和本地要执行的任务是提前有对应关系的，see TODO
    Strategy,           // 策略型任务，一般是一系列固定任务的集合

    /* 目前限制了任务类型最多有8种，可以满足正常使用了 */
    MaximumType = 7
};

/**
 * @brief The TaskStatus enum
 * @note  任务在准备处理前应先发出Ready状态的信号，在结束前应给出Success/Failure信号
 */
enum TaskStatus {
    Received = 0,       // 刚收到任务
    Ready,              // 准备执行
    InProgress,         // 执行中
    Success,            // 执行成功
    Failure             // 执行失败
};

// TODO 需要考虑扩展性，以后可能增加更多的字段
struct Task {
    TaskType type;
    int id;
    QString name;
    QString command;
};
Q_DECLARE_METATYPE(Task)

bool operator<(const Task& a, const Task &other);
QDBusArgument &operator<<(QDBusArgument&, const Task&);
const QDBusArgument &operator>>(const QDBusArgument&, Task&);

#endif // TASK_H
