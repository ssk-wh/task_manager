#include <QApplication>
#include <QDBusInterface>
#include <QRandomGenerator>
#include <QDebug>
#include <QDBusReply>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QDBusInterface inter("io.github.ssk.Manager", "/Manager", "io.github.ssk.Manager", QDBusConnection::sessionBus());
    int type = -1;
    int id = 0;
    QString name, command;

    for (int i = 0; i < 10; ++i) {
        // Ping type task
        type = 1;
        id++;
        name = "Ping";
        command = "Ping task";
        inter.call("dispatchTask", type, id, name, command);

        // Invalid task dispatch
        type = 0;
        id ++;
        name = QString("Invalid task");
        command = "bash -c \"sleep 3\"";
        inter.call("dispatchTask", type, id, name, command);

        int value = QRandomGenerator::global()->bounded(20);
        // Bash task dispatch (time-consuming)
        type = 2;
        id ++;
        name = QString("Time-consuming task %1").arg(i + 1);
        command = QString("bash -c \"sleep %1\"").arg(value);
        inter.call("dispatchTask", type, id, name, command);

        // Bash task dispatch (non-time-consuming)
        type = 2;
        id ++;
        name = "Bash task";
        command = "bash -c \"ls -al\"";
        inter.call("dispatchTask", type, id, name, command);

        // Settings task dispatch
        type = 3;
        id ++;
        name = "Settings task";
        command = "2";
        inter.call("dispatchTask", type, id, name, command);

        // Strategy task dispatch
        type = 4;
        id ++;
        name = "Strategy task";
        command = "";
        inter.call("dispatchTask", type, id, name, command);
    }

    return a.exec();
}
