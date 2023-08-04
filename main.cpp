#include "task_manager.h"

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusError>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(type);

    static QMap<QtMsgType, const char *> typeMap = {{QtMsgType::QtDebugMsg, "Debug"},
                                                    {QtMsgType::QtInfoMsg, "Info"},
                                                    {QtMsgType::QtWarningMsg, "Warning"},
                                                    {QtMsgType::QtCriticalMsg, "Critical"},
                                                    {QtMsgType::QtFatalMsg, "Fatal"}};
#ifdef QT_DEBUG
    QStringList list = msg.split(",");
    QString str;
    for (int i = 0; i < list.size(); ++i) {
        str.append(list[i]);
        str.append("\t");
    }
#endif

    fprintf(stderr, "[%s] [%s] [%s] %s \n"
            , QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz").toStdString().data()
            , QString("%1").arg(context.category, 20).toStdString().data()
            , QString("%1").arg(typeMap[type], 10).toStdString().data()
#ifdef QT_DEBUG
            , str.toLocal8Bit().constData());
#else
            , msg.toLocal8Bit().constData());
#endif
}
int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);
    QApplication a(argc, argv);
    a.setApplicationName("task_manager");

    qDebug() << "Application Start Running...";

    QDBusConnection connection = QDBusConnection::sessionBus();
    if(!connection.registerService("io.github.ssk-wh.TaskManager")) {
        qWarning() << "service is running, quit...";
        return 1;
    }

    TaskManager w;
    if (!connection.registerObject("/TaskManager", &w,QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllProperties | QDBusConnection::ExportAllSignals)) {
        qDebug() << connection.lastError().message();
        qWarning() << "service register failed, quit...";
        return 1;
    }

    return a.exec();
}
