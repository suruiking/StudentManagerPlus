#ifndef FINANACIALWIDGET_H
#define FINANACIALWIDGET_H

#include <QWidget>
#include<QChartView>
#include<QDate>
#include<QMap>
QT_CHARTS_USE_NAMESPACE

namespace Ui {
class FinanacialWidget;
}

class FinanacialWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FinanacialWidget(QWidget *parent = nullptr);
    ~FinanacialWidget();
    void initUi();//初始化界面
    void initConnections();//初始化连接
    //把学生丢入下拉框
    void loadStudents();
    //把财务记录显示到表格
    void loadRecords();
    //刷新两个表格
    void refreshCharts();

     void showEvent(QShowEvent *event) override;


private slots:
     //添加按钮
    void on_addButton_clicked();

    void on_deleteButton_clicked();

    void on_editButton_clicked();

private:
    Ui::FinanacialWidget *ui;
    QMap<QString, double> payTypeSum;  // 支付方式 → 金额  饼状图
    QMap<QDate,   double> dateSum;     // 日期 → 金额      折线图
};

#endif // FINANACIALWIDGET_H
