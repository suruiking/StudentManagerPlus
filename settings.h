#ifndef SETTINGS_H
#define SETTINGS_H
#include<QSettings>

class Settings
{
public:
    Settings();
    static Settings&instance();//获取单例实例的接口
    //把 Settings 类内部的 QSettings 对象返回出来给外部用
    QSettings&getQSettings(){return settings;}
   QString getDatabasePath() const;  // 获取数据库路径
    void setDatabasePath(const QString& path);  // 设置数据库路径
    bool getCacheEnabled() const;  // 获取登录缓存是否启用
    void setCacheEnabled(bool enabled);  // 设置登录缓存启用状态
    QString getLastUser() const;  // 获取上次登录的用户名
    void setLastUser(const QString& user);  // 设置上次登录的用户名


private:
    QSettings settings;
};

#endif // SETTINGS_H
