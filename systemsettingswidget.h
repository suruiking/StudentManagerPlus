#ifndef SYSTEMSETTINGSWIDGET_H
#define SYSTEMSETTINGSWIDGET_H

#include <QWidget>

namespace Ui {
class SystemSettingsWidget;
}

class SystemSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SystemSettingsWidget(QWidget *parent = nullptr);
    ~SystemSettingsWidget();
    void initUi();                 // 初始化控件属性（密文、只读等）
    //回显
    void loadSettings();           // 加载现有设置

    //更新密码
    bool validatePasswordChange(); // 校验密码修改
    void updatePassword();         // 真正执行密码更新

private slots:
    void on_browseButton_clicked();

    //保存
    void on_saveButton_clicked();

private:
    Ui::SystemSettingsWidget *ui;
};

#endif // SYSTEMSETTINGSWIDGET_H
