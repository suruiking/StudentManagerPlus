#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include<QsqlDatabase>

class DataBaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DataBaseManager(QObject *parent = nullptr);
    static DataBaseManager&instance();//单例接口
    void closeDatabse();
    bool openDatabase(const QString &path);
    QString getDatabsePath()const;
    void setDatabsePath(const QString&path);
    //初始化表格
    void initTables();

private:
    QSqlDatabase db;
    QString dbPath;

};

#endif // DATABASEMANAGER_H
