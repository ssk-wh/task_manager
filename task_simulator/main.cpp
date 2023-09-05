#include <QApplication>
#include <QDBusInterface>
#include <QRandomGenerator>
#include <QDebug>
#include <QDBusReply>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QDBusInterface inter("io.github.ssk.TaskManager", "/TaskManager", "io.github.ssk.TaskManager", QDBusConnection::sessionBus());
    int type = -1;
    int id = 0;
    QString name, command;

    for (int i = 0; i < 10; ++i) {
        // Ping类型的任务
        type = 1;
        id++;
        name = "Ping";
        command = "Ping任务";
        inter.call("dispatchTask", type, id, name, command);

        // 无效任务分发
        type = 0;
        id ++;
        name = QString("无效任务");
        command = "bash -c \"sleep 3\"";
        inter.call("dispatchTask", type, id, name, command);

        int value = QRandomGenerator::global()->bounded(20);
        // Bash任务分发 (耗时)
        type = 2;
        id ++;
        name = QString("耗时任务%1").arg(i + 1);
        command = QString("bash -c \"sleep %1\"").arg(value);
        inter.call("dispatchTask", type, id, name, command);

        // Bash任务分发 (非耗时)
        type = 2;
        id ++;
        name = "Bash任务";
        command = "bash -c \"ls -al\"";
        inter.call("dispatchTask", type, id, name, command);

        // Settings任务分发
        type = 3;
        id ++;
        name = "Settings任务";
        command = "2";
        inter.call("dispatchTask", type, id, name, command);

        // Stragety任务分发
        type = 4;
        id ++;
        name = "Stragety任务";
        command = "";
        inter.call("dispatchTask", type, id, name, command);
    }

    return a.exec();
}
