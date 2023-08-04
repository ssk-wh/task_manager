#include "settings_task_handler.h"

#include <QLoggingCategory>
#include <QProcess>
#include <QGSettings/QGSettings>

Q_LOGGING_CATEGORY(settingsTaskHandler, "handler-settings")

bool downloadWallpaperAndOpen()
{
    qCDebug(settingsTaskHandler) << "Get wallpaper and set";

    QProcess p;
    p.setWorkingDirectory("/tmp");
    p.start("/bin/bash -c \"wget -O wallpaper.png https://ssk-wh.github.io/2023/0436ca148/004.png\"");
    p.waitForStarted();
    p.waitForFinished();

    if (p.exitCode() != QProcess::NormalExit) {
        qCWarning(settingsTaskHandler) << "Get wallpaper failed, output:" << QString::fromUtf8(p.readAllStandardOutput())
                                   << "error:" << QString::fromUtf8(p.readAllStandardError());;
        return false;
    }

    QProcess::startDetached("/bin/bash -c \"xdg-open /tmp/wallpaper.png\"");

    return true;
}

bool setDockDisplayModeToFashion()
{
    qCDebug(settingsTaskHandler) << "Get dock display mode and set";

    QGSettings gsettings("com.deepin.dde.dock");
    if (!gsettings.keys().contains("displayMode")) {
        qCWarning(settingsTaskHandler) << "GSettings key: display-mode not found";
        return false;
    }

    QString mode = gsettings.get("displayMode").toString();
    if (mode != "efficient") {
       gsettings.set("displayMode", "efficient");
    }

    return true;
}

bool openTerminalAndRunTop()
{
    QProcess::startDetached("/bin/bash -c \"deepin-terminal -e top\" 1> /tmp/.task_manager_process.log 2>&1");
    return true;
}

SettingsTaskHandler::SettingsTaskHandler(const Task &info, QObject *parent)
    : AbstractTaskHandler(parent)
    , m_info(info)
{
    Q_ASSERT(m_info.type == TaskType::Settings);

    // Protocol agreed with the server (currently rather rough, suitable for some complex but frequently used operations).
    // For example, if the command from the server is "1", we download the wallpaper from the server and display it.
    m_settorMap.insert("1", &downloadWallpaperAndOpen);
    m_settorMap.insert("2", &setDockDisplayModeToFashion);
    m_settorMap.insert("3", &openTerminalAndRunTop);
}

void SettingsTaskHandler::run()
{
    Q_EMIT taskStatusChanged(m_info.id, TaskStatus::Ready);

    if (!m_settorMap.contains(m_info.command)){
        qCWarning(settingsTaskHandler) << "The task command is not recognized. The currently supported task commands are as follows:" << m_settorMap.values();
        Q_EMIT taskStatusChanged(m_info.id, TaskStatus::Failure);
    } else {
        auto settor = m_settorMap[m_info.command];
        Q_ASSERT(settor);
        bool result = (*settor)();
        if (result) {
            Q_EMIT taskStatusChanged(m_info.id, TaskStatus::Success);
        } else {
            Q_EMIT taskStatusChanged(m_info.id, TaskStatus::Failure);
        }
    }

    Q_EMIT taskHandleFinished(m_info.id);
}
