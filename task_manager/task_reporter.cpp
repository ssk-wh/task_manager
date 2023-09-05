#include "task_reporter.h"

#include <QThread>
#include <QLoggingCategory>
#include <QDebug>

#ifdef RANDOM_UPLOAD_STATUS_FAILED
#include <QRandomGenerator>
#endif

Q_LOGGING_CATEGORY(taskReporter, "taskreporter")

TaskReporter::TaskReporter(const QMap<int, TaskStatus> &taskMap, QObject *parent)
    : QObject(parent)
    , m_taskMap(taskMap)
{

}

void TaskReporter::run()
{
    QMapIterator<int, TaskStatus> it(m_taskMap);
    while (it.hasNext()) {
        it.next();
        int taskId = it.key();
        TaskStatus status = it.value();

        // 确保任务状态为成功或失败
        Q_ASSERT_X(status == TaskStatus::Success || status == TaskStatus::Failure, "Task status", "task status should be `Success|Failure`");

        // 模拟上传被阻塞的情况
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

        // 发送上传成功或失败信号
        if (success) {
            Q_EMIT uploadSuccess(taskId);
        } else {
            Q_EMIT uploadFailed(taskId);
        }
    }

    // 发送上传完成信号
    Q_EMIT uploadFinished();
}
