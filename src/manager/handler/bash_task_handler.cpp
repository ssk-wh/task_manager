#include "bash_task_handler.h"

#include <QDebug>
#include <QLoggingCategory>
#include <QThread>
#include <QProcess>

Q_LOGGING_CATEGORY(baskTaskHandler, "handler-bash")

// Limit the maximum execution time of a single task to 2 hours
#define MAX_PROCESS_TIMEOUT (2 * 60 * 60 * 1000)
BashTaskHandler::BashTaskHandler(const Task &info, QObject *parent)
    : AbstractTaskHandler(parent)
    , m_info(info)
{
    Q_ASSERT(m_info.type == TaskType::Bash);
}

void BashTaskHandler::run()
{
    Q_EMIT taskStatusChanged(m_info.id, TaskStatus::Ready);

    QProcess p;
    p.start(m_info.command);
    p.waitForStarted();
    p.waitForFinished(MAX_PROCESS_TIMEOUT);
    if (p.exitCode() == QProcess::NormalExit) {
        Q_EMIT taskStatusChanged(m_info.id, TaskStatus::Success);
    } else {
        qCWarning(baskTaskHandler) << "Task failed, output:" << QString::fromUtf8(p.readAllStandardOutput())
                                   << "error:" << QString::fromUtf8(p.readAllStandardError());
        Q_EMIT taskStatusChanged(m_info.id, TaskStatus::Failure);
    }

    Q_EMIT taskHandleFinished(m_info.id);
}
