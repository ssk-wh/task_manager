#ifndef SETTINGS_TASK_HANDLER_H
#define SETTINGS_TASK_HANDLER_H

#include "abstract_task_handler.h"

/**
 * @brief The SettingsTaskHandler class
 * @note  获取服务端特有配置，并设置到当前电脑上，比如壁纸，电脑显示模式等设置
 */
class SettingsTaskHandler : public AbstractTaskHandler
{
    Q_OBJECT

public:
    explicit SettingsTaskHandler(const Task &info, QObject *parent = nullptr);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    Task m_info;

    typedef bool(*Settor)();
    QMap<QString, Settor> m_settorMap;
};

DECLARE_CREATE(SettingsTaskHandler);

#endif // SETTINGS_TASK_HANDLER_H
