#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QDBusMetaType>

enum TaskType {
    Unknown = 0,
    Ping,               // Check if the client is alive; should respond to the server in the agreed way upon receipt
    Bash,               // The command content is usually a bash statement
    Settings,           // The command content corresponds to a local task to be executed, see TODO
    Strategy,           // Strategy-type task, usually a collection of fixed tasks

    /* Currently, the number of task types is limited to 8, which should be sufficient for normal use */
    MaximumType = 7
};

/**
 * @brief The TaskStatus enum
 * @note  Before processing, a Ready status signal should be emitted; before finishing, a Success/Failure signal should be given
 */
enum TaskStatus {
    Received = 0,       // Task just received
    Ready,              // Ready to execute
    InProgress,         // Executing
    Success,            // Executed successfully
    Failure             // Execution failed
};

// TODO: Consider extensibility, more fields may be added in the future
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
