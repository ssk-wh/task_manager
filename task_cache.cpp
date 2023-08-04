#include "task_cache.h"

#include <QLoggingCategory>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>

Q_LOGGING_CATEGORY(taskCache, "taskcache")

#define TASK_FILE(task_id) (m_configPath + QString("/%1").arg(task_id))

TaskCache::TaskCache(QObject *parent)
    : QObject(parent)
    , m_configPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation))
{
    QDir dir(m_configPath);
    if (!dir.exists()) {
        dir.mkpath(m_configPath);
    }
}

void TaskCache::add(const TaskInfo &info, const TaskStatus &status)
{
    qCDebug(taskCache) << "Add to cache, id:" << info.task_id;

    QJsonObject obj;
    obj["id"] = info.task_id;
    obj["type"] = info.task_type;
    obj["command"] = info.task_command;
    obj["status"] = status;

    QJsonDocument doc(obj);
    QByteArray taskContent = doc.toJson();

    QFile file(TASK_FILE(info.task_id));
    if (Q_UNLIKELY(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))) {
        qCWarning(taskCache) << "Write to cache failed, id:" << info.task_id;
        file.close();
        return;
    }

    file.write(taskContent);
    file.close();
}

void TaskCache::update(int taskId, const TaskStatus &status)
{
    qCDebug(taskCache) << "Update cache, id:" << taskId << ", status:" << status;

    QFile file(TASK_FILE(taskId));
    if (Q_UNLIKELY(!file.open(QIODevice::ReadWrite | QIODevice::Text))) {
        qCWarning(taskCache) << "Failed to update cache, id:" << taskId;
        file.close();
        return;
    }

    QTextStream stream(&file);
    QString taskContent = stream.readAll();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(taskContent.toUtf8(), &err);
    if (Q_UNLIKELY(err.error != QJsonParseError::NoError)) {
        qCWarning(taskCache) << "Failed to update cache, id:" << taskId << ", error:" << err.errorString();
        file.close();
        return;
    }

    Q_ASSERT(doc.isObject());

    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        obj["status"] = status;
        QJsonDocument newDoc(obj);
        file.resize(0);
        stream << newDoc.toJson();
    } else {
        qCWarning(taskCache) << "Failed to update cache, format not match";
    }

    file.close();
}

void TaskCache::remove(int taskId)
{
    qCDebug(taskCache) << "Remove from cache, id:" << taskId;

    const QString &fileName = TASK_FILE(taskId);
    if (QFile::exists(fileName)) {
        QFile::remove(fileName);
    }
}

const QMap<TaskInfo, TaskStatus> &TaskCache::loadFromCache()
{
    m_historyTaskMap.clear();

    QDir dir(m_configPath);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    const QFileInfoList &fileList = dir.entryInfoList();
    for (const QFileInfo &fileInfo : fileList) {
        QFile file(fileInfo.absoluteFilePath());
        if (Q_UNLIKELY(!file.open(QIODevice::ReadOnly))) {
            qCWarning(taskCache) << "Failed to open file:" << fileInfo.absoluteFilePath();
            file.close();
            continue;
        }

        auto task_content = file.readAll();
        file.close();

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(task_content, &err);
        if (Q_UNLIKELY(err.error != QJsonParseError::NoError)) {
            qCWarning(taskCache) << "Failed to read json:" << fileInfo.absoluteFilePath() << ", error:" << err.errorString();
            continue;
        }

        Q_ASSERT(doc.isObject());
        if (!doc.isObject()) {
            qCWarning(taskCache) << "Failed to read, format not match:" << fileInfo.absoluteFilePath();
            continue;
        }

        auto obj = doc.object();
        TaskInfo info;
        info.task_id = obj["id"].toInt();
        info.task_type = static_cast<TaskType>(obj["type"].toInt());
        info.task_command = obj["command"].toString();
        TaskStatus status = static_cast<TaskStatus>(obj["status"].toInt());

        m_historyTaskMap.insert(info, status);
    }

    return m_historyTaskMap;
}
