#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include<QLineEdit>
#include<QPushButton>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
    //确保默认的admin用户
    void ensureDefaultAdminUser();
    //验证用户名和密码
    bool validateUser(const QString &username,const QString &password);
    QString hashPassword(const QString &password);         // Sha256
    QString encryptPassword(const QString &password);      // 本地缓存加密
    QString decryptPassword(const QString &encrypted);     // 本地缓存解密
    bool loadCredentials(QString &username,QString &password); // 从配置文件加载缓存
    void saveCredentials(const QString &username,const QString &password);// 保存 / 清理缓存


private slots:
    void on_loginButton_clicked();

    void on_pushButton_clicked();

private:
    Ui::LoginDialog *ui;

};

#endif // LOGINDIALOG_H
