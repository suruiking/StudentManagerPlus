#include "mainwindow.h"
#include"databasemanager.h"
#include"logindialog.h"

#include <QApplication>
#include<QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
     QFile f(":/qss/wechat_fresh.qss");
    if (f.open(QFile::ReadOnly)) {
        a.setStyleSheet(f.readAll());
        f.close();
    }
    DataBaseManager::instance();
    LoginDialog login;
    if(login.exec()==QDialog::Accepted){
        MainWindow w;
        w.show();
        return a.exec();
    }else
        return 0;

}
