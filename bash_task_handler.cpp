#include "bash_task_handler.h"

#include <QDebug>
#include <QLoggingCategory>

#include <QThread>
#include <QProcess>

Q_LOGGING_CATEGORY(baskTaskHandler, "taskhandler-bash")

BashTaskHandler::BashTaskHandler(const TaskInfo &info, QObject *parent)
    : AbstractTaskHandler(parent)
    , m_info(info)
{

}

void BashTaskHandler::run()
{
    if (m_info.task_type != TaskType::Bash)
        return;

    Q_EMIT taskStatusChanged(m_info.task_id, TaskStatus::Ready);

    QProcess p;
    p.start(m_info.task_command);
    p.waitForStarted();
    p.waitForFinished();
    if (p.exitCode() == 0) {
        Q_EMIT taskStatusChanged(m_info.task_id, TaskStatus::Success);
    } else {
        qCWarning(baskTaskHandler) << "Task failed, output:" << QString::fromUtf8(p.readAllStandardOutput())
                                   << "error:" << QString::fromUtf8(p.readAllStandardError());
        Q_EMIT taskStatusChanged(m_info.task_id, TaskStatus::Failure);
    }

    Q_EMIT taskHandleFinished(m_info.task_id);
}
