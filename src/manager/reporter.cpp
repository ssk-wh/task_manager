#include "reporter.h"

#include <QThread>
#include <QLoggingCategory>
#include <QDebug>

#ifdef RANDOM_UPLOAD_STATUS_FAILED
#include <QRandomGenerator>
#endif

Q_LOGGING_CATEGORY(taskReporter, "taskreporter")

Reporter::Reporter(const QMap<int, TaskStatus> &taskMap, QObject *parent)
    : QObject(parent)
    , m_taskMap(taskMap)
{

}

void Reporter::run()
{
    QMapIterator<int, TaskStatus> it(m_taskMap);
    while (it.hasNext()) {
        it.next();
        int taskId = it.key();
        TaskStatus status = it.value();

        // Ensure the task status is Success or Failure
        Q_ASSERT_X(status == TaskStatus::Success || status == TaskStatus::Failure, "Task status", "task status should be `Success|Failure`");

        // Simulate upload being blocked
#ifdef SIMULATE_UPLOAD_BLOCKED
        QThread::msleep(100);
#endif

        bool success = false;
#ifdef RANDOM_UPLOAD_STATUS_FAILED
        quint32 value = QRandomGenerator::global()->generate() % 10;
        if (value > 3) {
            success = true;
        }
#endif

        // Emit upload success or failure signal
        if (success) {
            Q_EMIT uploadSuccess(taskId);
        } else {
            Q_EMIT uploadFailed(taskId);
        }
    }

    // Emit upload finished signal
    Q_EMIT uploadFinished();
}
