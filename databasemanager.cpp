#include "databasemanager.h"
#include<QDebug>
#include<QSqlError>
#include<QSqlQuery>

DataBaseManager::DataBaseManager(QObject *parent)
    : QObject{parent}
{
    db=QSqlDatabase::addDatabase("QSQLITE");
    dbPath = "Manager.db" ;

    if (!openDatabase(dbPath)) {
        qDebug() << "无法打开数据库:" << db.lastError().text();
    }

}
//单例模式
DataBaseManager &DataBaseManager::instance()
{
    //已经创建了一个对象
    static DataBaseManager inst;
    return inst;

}
//打开数据库
bool DataBaseManager::openDatabase(const QString &path)
{
    db.setDatabaseName(path);
    if(!db.open()){
        qDebug()<<"无法打开数据库："<<db.lastError().text();
        return false;

    }else{
        qDebug()<<"数据库打开成功";
        initTables();
        return true;
    }


}

//关闭数据库
void DataBaseManager::closeDatabse()
{
    if(db.isOpen()){
        db.close();
    }

}

void DataBaseManager::initTables()
{
    QSqlQuery q;
    // studentInfo
    q.exec("CREATE TABLE IF NOT EXISTS studentInfo ("
           "id TEXT PRIMARY KEY,"
           "name TEXT,"
           "gender TEXT,"
           "birthday TEXT,"
           "join_date TEXT,"
           "study_goal TEXT,"
           "progress TEXT,"
           "photo BLOB"
           ")");

    // financialRecords
    q.exec("CREATE TABLE IF NOT EXISTS financialRecords ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "student_id TEXT NOT NULL,"
           "payment_date TEXT,"
           "amount REAL,"
           "payment_type TEXT,"
           "notes TEXT"
           ")");

    // schedule
    q.exec("CREATE TABLE IF NOT EXISTS schedule ("
           "date TEXT NOT NULL,"
           "time TEXT NOT NULL,"
           "course_name TEXT,"
           "PRIMARY KEY (date, time)"
           ")");

    // honorWall
    q.exec("CREATE TABLE IF NOT EXISTS honorWall ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "image_data BLOB,"
           "description TEXT,"
           "added_date TEXT"
           ")");

    // users
    q.exec("CREATE TABLE IF NOT EXISTS users ("
           "username TEXT PRIMARY KEY,"
           "password TEXT NOT NULL"
           ")");

}

//获得当前数据库的路径
QString DataBaseManager::getDatabsePath() const
{
    return dbPath;
}

//设置新的路径
void DataBaseManager::setDatabsePath(const QString &path)
{
    if(path==dbPath)
        return;
    dbPath=path;
    closeDatabse();
    if(!openDatabase(dbPath)){
        qDebug()<<"切换数据库失败:"<<db.lastError().text();
    }

}
