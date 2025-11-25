#include "settings.h"

Settings::Settings():settings("config.ini",QSettings::IniFormat)//默认为ini格式
{
    //初始化
}

Settings &Settings::instance()
{
    static Settings instance;  // 局部静态变量，确保仅初始化一次
    return instance;

}
//获取数据库路径
QString Settings::getDatabasePath() const
{
    return settings.value("Database/Path", "Manager.db").toString();

}
//设置数据库路径
void Settings::setDatabasePath(const QString &path)
{
    settings.setValue("Database/Path",path);
}

//获取上次有没有勾选记住密码
bool Settings::getCacheEnabled() const
{
    return settings.value("Login/CacheEnabled",true).toBool();

}

void Settings::setCacheEnabled(bool enabled)
{
    settings.setValue("Login/CacheEnabled",enabled);
}

//获取上次登录的用户名
QString Settings::getLastUser() const
{
    return settings.value("Login/LastUser","").toString();
}

void Settings::setLastUser(const QString &user)
{
    settings.setValue("Login/LastUser",user);
}
