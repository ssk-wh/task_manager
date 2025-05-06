#include "manager.h"
#include "taskmanager_adapter.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QLoggingCategory>

#ifdef ENABLE_DBUS_REGISTER
#include <QDBusConnection>
#include <QDBusError>
#endif

Q_LOGGING_CATEGORY(main_logging, "main")

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(type);

    static QMap<QtMsgType, const char *> typeMap = {{QtMsgType::QtDebugMsg, "Debug"},
                                                    {QtMsgType::QtInfoMsg, "Info"},
                                                    {QtMsgType::QtWarningMsg, "Warning"},
                                                    {QtMsgType::QtCriticalMsg, "Critical"},
                                                    {QtMsgType::QtFatalMsg, "Fatal"}};
    QStringList list = msg.split(",");
    QString str;
    for (auto s : list) {
        str += QString("%1").arg(s[0].toUpper() + s.mid(1), -30);
    }

    fprintf(stderr, "[%s] [%s] [%s] %s \n"
            , qPrintable(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"))
            , qPrintable(QString("%1").arg(context.category, -18))
            , qPrintable(QString("%1").arg(typeMap[type], -10))
            , str.toLocal8Bit().constData());
}
int main(int argc, char *argv[])
{
#ifdef QT_DEBUG
    //    qunsetenv("QT_LOGGING_RULES");
    //    QLoggingCategory::setFilterRules(QStringLiteral("taskcache.debug=false"));
#endif

    qInstallMessageHandler(myMessageOutput);
    QCoreApplication a(argc, argv);
    a.setApplicationName("task-manager");

    qCDebug(main_logging) << "Application Start Running...";

#ifdef ENABLE_DBUS_REGISTER
    QDBusConnection connection = QDBusConnection::sessionBus();
    if(!connection.registerService("io.github.ssk.TaskManager")) {
        qCWarning(main_logging) << "Service is running, quit...";
        return 1;
    }
#endif

    Manager w;

#ifdef ENABLE_DBUS_REGISTER
    new TaskManagerAdaptor(&w);
    if (!connection.registerObject("/TaskManager", &w)) {
        qDebug() << connection.lastError().message();
        qCWarning(main_logging) << "Service register failed, quit...";
        return 1;
    }
    qDebug() << "DBus register success";
#endif

    return a.exec();
}
