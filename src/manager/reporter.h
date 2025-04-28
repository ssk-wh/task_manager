#ifndef REPORTER_H
#define REPORTER_H

#include "task.h"

#include <QObject>
#include <QRunnable>

/**
 * @brief Task Reporter class
 * 
 * This class is used to upload task execution status in a separate thread.
 */

class Reporter : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit Reporter(const QMap<int, TaskStatus> &map = QMap<int, TaskStatus>(), QObject *parent = nullptr);

Q_SIGNALS:
    void uploadSuccess(int taskId);
    void uploadFailed(int taskId);
    void uploadFinished();

protected:
    void run() Q_DECL_OVERRIDE;

private:
    QMap<int, TaskStatus> m_taskMap;
};

#endif // REPORTER_H
