/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp io.github.ssk.TaskManager.xml -a task_manager_adapter
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef TASK_MANAGER_ADAPTER_H
#define TASK_MANAGER_ADAPTER_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface io.github.ssk.TaskManager
 */
class TaskManagerAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.github.ssk.TaskManager")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"io.github.ssk.TaskManager\">\n"
"    <method name=\"dispatchTask\">\n"
"      <arg direction=\"in\" type=\"i\" name=\"type\"/>\n"
"      <arg direction=\"in\" type=\"i\" name=\"id\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"name\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"command\"/>\n"
"    </method>\n"
"    <signal name=\"taskStatusChanged\">\n"
"      <arg type=\"i\" name=\"taskId\"/>\n"
"      <arg type=\"i\" name=\"status\"/>\n"
"    </signal>\n"
"    <signal name=\"taskHandleFinished\">\n"
"      <arg type=\"i\" name=\"taskId\"/>\n"
"    </signal>\n"
"  </interface>\n"
        "")
public:
    TaskManagerAdaptor(QObject *parent);
    virtual ~TaskManagerAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void dispatchTask(int type, int id, const QString &name, const QString &command);
Q_SIGNALS: // SIGNALS
    void taskHandleFinished(int taskId);
    void taskStatusChanged(int taskId, int status);
};

#endif
