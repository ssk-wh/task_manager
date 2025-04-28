#include "cache.h"

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

Cache::Cache(QObject *parent)
    : QObject(parent)
    , m_configPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation))
{
    QDir dir(m_configPath);
    if (!dir.exists()) {
        dir.mkpath(m_configPath);
    }
}

QMap<Task, TaskStatus> Cache::loadFromCache() const
{
    QMap<Task, TaskStatus> historyTaskMap;

    QDir dir(m_configPath);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    const QFileInfoList &fileList = dir.entryInfoList();
    for (const QFileInfo &fileInfo : fileList) {
        QJsonObject obj = readJsonFromFile(fileInfo.absoluteFilePath());
        if (!obj.isEmpty()) {
            Task info;
            info.id = obj["id"].toInt();
            info.type = static_cast<TaskType>(obj["type"].toInt());
            info.name = obj["name"].toString();
            info.command = obj["command"].toString();
            TaskStatus status = static_cast<TaskStatus>(obj["status"].toInt());

            historyTaskMap.insert(info, status);
        }
    }

    return historyTaskMap;
}

void Cache::add(const Task &info, const TaskStatus &status)
{
    qCDebug(taskCache) << "Add task info to cache, id:" << info.id;

    QJsonObject obj;
    obj["id"] = info.id;
    obj["type"] = static_cast<int>(info.type);
    obj["name"] = info.name;
    obj["command"] = info.command;
    obj["status"] = static_cast<int>(status);

    writeJsonToFile(TASK_FILE(info.id), obj);

#ifdef RECORD_ALL_TASK_INFO
    // Keep details of the most recent 1000 tasks
    m_taskList.append({info, status, QDateTime::currentDateTime(), QDateTime(), QDateTime()});
    if (m_taskList.size() > 10000)
        m_taskList.pop_front();
#endif
}

void Cache::update(int taskId, const TaskStatus &status)
{
    static QMap<TaskStatus, QString> map = {{TaskStatus::Ready, "Ready"}
                                            , {TaskStatus::Failure, "Failure"}
                                            , {TaskStatus::Success, "Success"}
                                            , {TaskStatus::InProgress, "InProgress"}};


#ifdef RECORD_ALL_TASK_INFO
    QString name;
    for (auto &d : m_taskList) {
        if (d.info.id != taskId)
            continue;

        d.status = status;
        name = d.info.name;

        if (status == TaskStatus::Ready) {
            d.startTime = QDateTime::currentDateTime();
        }
        break;
    }
    qCDebug(taskCache) << "Task status changed, id:" << taskId << ", status:" << map[status] << ", name:" << name;
#else
    qCDebug(taskCache) << "Task status changed, id:" << taskId << ", status:" << map[status];
#endif

    if (Q_UNLIKELY(!QFile::exists(TASK_FILE(taskId)))) {
        qCWarning(taskCache) << "Error from internal, file not exists:" << TASK_FILE(taskId);
        return;
    }

    QJsonObject obj = readJsonFromFile(TASK_FILE(taskId));
    if (!obj.isEmpty()) {
        obj["status"] = static_cast<int>(status);
        writeJsonToFile(TASK_FILE(taskId), obj);
    } else {
        qCWarning(taskCache) << "Failed to update cache, format not match";
    }
}

void Cache::remove(int taskId)
{
    qCDebug(taskCache) << "Remove from cache, id:" << taskId;

#ifdef RECORD_ALL_TASK_INFO
    for (auto &d : m_taskList) {
        if (d.info.id != taskId)
            continue;

        d.endTime = QDateTime::currentDateTime();
        break;
    }

#endif

    const QString &fileName = TASK_FILE(taskId);
    if (QFile::exists(fileName)) {
        QFile::remove(fileName);
    }
}

void Cache::writeJsonToFile(const QString &fileName, const QJsonObject &obj) const
{
    QJsonDocument doc(obj);
    QByteArray taskContent = doc.toJson();

    QFile file(fileName);
    if (Q_UNLIKELY(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))) {
        qCWarning(taskCache) << "Write to cache failed, file:" << fileName;
        return;
    }

    file.write(taskContent);
    file.close();
}

QJsonObject Cache::readJsonFromFile(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(taskCache) << "Failed to open file:" << fileName;
        return QJsonObject();
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        qCWarning(taskCache) << "Failed to read json:" << fileName << ", error:" << err.errorString();
        return QJsonObject();
    }

    if (!doc.isObject()) {
        qCWarning(taskCache) << "Failed to read, format not match:" << fileName;
        return QJsonObject();
    }

    return doc.object();
}
