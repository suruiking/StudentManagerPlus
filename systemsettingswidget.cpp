#include "systemsettingswidget.h"
#include "ui_systemsettingswidget.h"
#include"settings.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>

SystemSettingsWidget::SystemSettingsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SystemSettingsWidget)
{
    ui->setupUi(this);
    initUi();
    loadSettings();
}

SystemSettingsWidget::~SystemSettingsWidget()
{
    delete ui;
}

//初始化ui
void SystemSettingsWidget::initUi()
{
    // 版本信息显示区只读
    ui->versionInfoTextEdit->setReadOnly(true);

    // 给版本信息一个默认文本（可随意改）
    ui->versionInfoTextEdit->setPlainText(
        "原神大学管理系统 1.0\n"
        "开发环境：Qt 6.6，Qt Creator 12.0.2，Windows 10"
        );
}

//在ui上面显示
void SystemSettingsWidget::loadSettings()
{
    auto &s = Settings::instance();

    //回显数据库路径
    ui->dbPathLineEdit->setText(s.getDatabasePath());
    //回显“是否启用登录缓存”
    ui->cacheCheckBox->setChecked(s.getCacheEnabled());
}

//更改密码，validate 是 “是否允许修改密码”的判断器
bool SystemSettingsWidget::validatePasswordChange()
{
    // 1. 新密码与确认密码一致性校验
    if (ui->newPasswordLineEdit->text() != ui->confirmPasswordLineEdit->text()) {
        QMessageBox::warning(this, "错误", "新密码与确认密码不一致");
        return false;
    }

    // 2. 获取当前登录用户
    QString currentUser = Settings::instance().getLastUser();
    if (currentUser.isEmpty()) {
        QMessageBox::warning(this, "错误", "未找到当前用户");
        return false;
    }

    // 3. 从数据库读取当前用户的密码哈希
    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE username = ?");
    query.addBindValue(currentUser);
    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "错误",
                              "数据库查询失败: " + query.lastError().text());
        return false;
    }
    //获得密码
    QString storedHash = query.value(0).toString();

    // 4. 对输入的旧密码做 Sha256，转为哈希值与数据库的对比
    QString inputHash = QString(
        QCryptographicHash::hash(
            ui->oldPasswordLineEdit->text().toUtf8(),
            QCryptographicHash::Sha256
            ).toHex()
        );

    //输入密码和旧的对比
    if (storedHash != inputHash) {
        QMessageBox::warning(this, "错误", "旧密码不正确");
        return false;
    }

    return true;
}
//在所有校验通过后，把新密码安全写入数据库。
void SystemSettingsWidget::updatePassword()
{
    if (!validatePasswordChange())
        return;

    // 生成新密码哈希
    QString newHash = QString(
        QCryptographicHash::hash(
            ui->newPasswordLineEdit->text().toUtf8(),
            QCryptographicHash::Sha256
            ).toHex()
        );

    QString currentUser = Settings::instance().getLastUser();

    QSqlQuery query;
    query.prepare("UPDATE users SET password = ? WHERE username = ?");
    query.addBindValue(newHash);
    query.addBindValue(currentUser);

    if (!query.exec()) {
        QMessageBox::critical(this, "错误",
                              "密码更新失败: " + query.lastError().text());
        return;
    }

    QMessageBox::information(this, "提示", "密码更新成功");

    // 清空输入框，免得留着有安全感太强
    ui->oldPasswordLineEdit->clear();
    ui->newPasswordLineEdit->clear();
    ui->confirmPasswordLineEdit->clear();
}

//浏览文件，选择新的路径
void SystemSettingsWidget::on_browseButton_clicked()
{
    QString path = QFileDialog::getSaveFileName(
        this,
        "选择数据库文件",
        ui->dbPathLineEdit->text(),
        "SQLite Databases (*.db *.sqlite)"
        );

    if (!path.isEmpty()) {
        ui->dbPathLineEdit->setText(path);
    }
}

//保存按钮
void SystemSettingsWidget::on_saveButton_clicked()
{
    //拿到单例
    auto &s = Settings::instance();

    //新旧地址
    QString oldDbPath = s.getDatabasePath();
    QString newDbPath = ui->dbPathLineEdit->text().trimmed();

    if (newDbPath.isEmpty()) {
        QMessageBox::warning(this, "错误", "数据库路径不能为空");
        return;
    }

    // 1. 保存数据库路径
    bool dbPathChanged = (newDbPath != oldDbPath);
    s.setDatabasePath(newDbPath);

    // 2. 保存“是否启用登录缓存”
    s.setCacheEnabled(ui->cacheCheckBox->isChecked());

    // 3. 如果用户输入了新密码，就尝试更新密码
    if (!ui->newPasswordLineEdit->text().isEmpty()
        || !ui->oldPasswordLineEdit->text().isEmpty()
        || !ui->confirmPasswordLineEdit->text().isEmpty()) {

        updatePassword();
    }

    // 4. 数据库路径变更提示（通常需要重启）
    if (dbPathChanged) {
        QMessageBox::information(this, "提示", "数据库路径修改将在重启后生效");
    } else {
        QMessageBox::information(this, "提示", "设置已保存");
    }
}

