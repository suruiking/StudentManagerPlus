#include "logindialog.h"
#include "ui_logindialog.h"
#include"settings.h"
#include<QMessageBox>
#include<QSqlQuery>
#include<QDebug>
#include<QSqlError>
#include<QCryptographicHash>


//密钥，做xor加密和还原的
static const QByteArray ENCRYPTION_KEY = "your_encryption_key";

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/res/tubiao.webp"));
    ui->lineEditPassword->setEchoMode(QLineEdit::Password);
    ensureDefaultAdminUser();

    bool cacheEnabled=Settings::instance().getCacheEnabled();
    ui->checkBoxRemember->setChecked(cacheEnabled);

    if(cacheEnabled){
        QString username,password;
        if(loadCredentials(username,password)){
            ui->lineEditUsername->setText(username);
            ui->lineEditPassword->setText(password);
        }
    }





}

LoginDialog::~LoginDialog()
{
    delete ui;
}

//确保有默认的admin
void LoginDialog::ensureDefaultAdminUser()
{
    QSqlQuery query;

    query.prepare("INSERT OR IGNORE INTO users(username, password) "
                  "VALUES(:username, :password)");
    query.bindValue(":username", "admin");
    query.bindValue(":password", hashPassword("123456"));  // 默认密码 123456

    if (!query.exec()) {
        qDebug() << "插入默认 admin 用户失败:" << query.lastError();
    }
}

//从数据库读出用户名的
bool LoginDialog::validateUser(const QString &username, const QString &password)
{
    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "登录查询失败:" << query.lastError();
        return false;
    }
    if (!query.next()) {
        // 用户不存在
        return false;
    }

    QString storedHash=query.value(0).toString();
    QString inputHash=hashPassword(password);
    return storedHash==inputHash;

}

QString LoginDialog::hashPassword(const QString &password)
{
     QByteArray data = password.toUtf8();
     QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    return QString::fromLatin1(hash.toHex());
}


//明文密码变为加密密码
QString LoginDialog::encryptPassword(const QString &password)
{
    if(ENCRYPTION_KEY.isEmpty())
        return password;
    //转为二进制
    QByteArray data=password.toUtf8();
    for(int i=0;i<data.size();i++){
        //异或加密
          data[i] = data[i] ^ ENCRYPTION_KEY.at(i % ENCRYPTION_KEY.size());
    }
    //转base64
    return QString::fromLatin1(data.toBase64());

}

//还原为明文密码
QString LoginDialog::decryptPassword(const QString &encrypted)
{
    QByteArray data = QByteArray::fromBase64(encrypted.toLatin1());
    for (int i = 0; i < data.size(); ++i) {
        data[i] = data[i] ^ ENCRYPTION_KEY.at(i % ENCRYPTION_KEY.size());
    }
    return QString::fromUtf8(data);
}

//加载记住的密码
bool LoginDialog::loadCredentials(QString &username, QString &password)
{
    //拿单例，然后拿到里面的qsettings
    auto &qs = Settings::instance().getQSettings();

    //读取缓存的
    username = qs.value("username").toString();
    const QString encryptedPassword = qs.value("password").toString();

    if (!username.isEmpty() && !encryptedPassword.isEmpty()) {
        password = decryptPassword(encryptedPassword);
        return true;
    }
    return false;
}
//保存密码，用户输入账号密码 → 点击登录 → 登录成功时保存
void LoginDialog::saveCredentials(const QString &username, const QString &password)
{

    auto&qs=Settings::instance().getQSettings();
    if(username.isEmpty()||password.isEmpty()){
        qs.remove("username");
        qs.remove("password");
        return;
    }
    qs.setValue("username",username);
    qs.setValue("password",encryptPassword(password));
    Settings::instance().setLastUser(username);
}

void LoginDialog::on_loginButton_clicked()
{
    QString username=ui->lineEditUsername->text();
    QString password=ui->lineEditPassword->text();
    if(username.isEmpty()||password.isEmpty()){
        QMessageBox::warning(this,"提示","用户名和密码都要填");
        return;
    }
    if(!validateUser(username,password)){
        QMessageBox::warning(this, "登录失败", "用户名或密码错误。");
        return;
    }
    if (ui->checkBoxRemember->isChecked()) {
        //保存密码
        saveCredentials(username, password);
        Settings::instance().setCacheEnabled(true);
    } else {
        saveCredentials(QString(), QString());  // 清空缓存
        Settings::instance().setCacheEnabled(false);
    }
    //关闭当前对话框，并把返回结果设置为 “Accepted（接受）
    accept();
}


void LoginDialog::on_pushButton_clicked()
{
    reject();
}

