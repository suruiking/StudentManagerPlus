#ifndef HONORWALLWIDGET_H
#define HONORWALLWIDGET_H

#include <QWidget>
#include<QMap>
#include<QDate>

namespace Ui {
class HonorWallWidget;
}
struct HonorItem
{
    int id = -1;
    QPixmap image;
    QString description;
    QDate addedDate;
};
class HonorWallWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HonorWallWidget(QWidget *parent = nullptr);
    ~HonorWallWidget();
    void initUi();
    void initConnections();
    void loadHonors();
    void showCurrentHonor();   // 根据当前选中项更新右侧预览

private slots:
    void on_addButton_clicked();

    void on_deleteButton_clicked();
    void on_honorListWidget_currentRowChanged(int row);

private:
    Ui::HonorWallWidget *ui;
    QVector<HonorItem> items_;

};

#endif // HONORWALLWIDGET_H
