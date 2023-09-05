#-------------------------------------------------
#
# Project created by QtCreator 2023-08-01T14:01:52
#
#-------------------------------------------------

# 允许重新执行以前未执行完毕的任务
DEFINES += ENABLE_LOAD_HISTORY_TASK

# 允许导出dbus服务
DEFINES += ENABLE_DBUS_REGISTER

CONFIG(debug, debug|release){
    # 模拟随机上传任务状态失败
    DEFINES += RANDOM_UPLOAD_STATUS_FAILED

    # 模拟上传任务状态阻塞的场景(如网络请求阻塞)
    DEFINES += SIMULATE_UPLOAD_BLOCKED

    # 记录所有任务信息
    DEFINES += RECORD_ALL_TASK_INFO
}

QT += core dbus

TARGET = task_manager
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11 link_pkgconfig
PKGCONFIG += gsettings-qt

system(qdbusxml2cpp io.github.ssk.TaskManager.xml -a task_manager_adapter)

SOURCES += \
        main.cpp \
    task_dispatch.cpp \
    task_manager.cpp \
    task_cache.cpp \
    task.cpp \
    task_manager_adapter.cpp \
    handler/bash_task_handler.cpp \
    handler/settings_task_handler.cpp \
    handler/strategy_task_handler.cpp \
    task_reporter.cpp \
    handler/ping_task_handler.cpp

HEADERS += \
    task_dispatch.h \
    task_manager.h \
    task_cache.h \
    task.h \
    task_manager_adapter.h \
    handler/abstract_task_handler.h \
    handler/bash_task_handler.h \
    handler/settings_task_handler.h \
    handler/strategy_task_handler.h \
    task_reporter.h \
    handler/ping_task_handler.h

INCLUDEPATH += handler \
    QGSettings

DISTFILES += \
    io.github.ssk.TaskManager.xml

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
