#ifndef SCHEDULEWIDGET_H
#define SCHEDULEWIDGET_H

#include <QWidget>
#include<QDate>

namespace Ui {
class ScheduleWidget;
}

class ScheduleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScheduleWidget(QWidget *parent = nullptr);
    ~ScheduleWidget();
    void initUi();//初始化界面
    void initWeekInfo();//初始化当前第几周，哪一年
    void updateWeekLabels();//更新日期范围
    void refreshTable();//刷新课程格子
    QDate dateForRow(int row) const;
    QString timeSlotKeyForColumn(int column) const;
    void updateCurrentMondayFromCombo();


private slots:
    void on_btnPrevWeek_clicked();

    void on_btnNextWeek_clicked();

    void on_btnAddCourse_clicked();

    void on_btnRemoveCourse_clicked();

    void on_comboBoxWeek_currentIndexChanged(int index);

    void on_comboBoxYear_currentIndexChanged(int index);

private:
    Ui::ScheduleWidget *ui;
    QDate currentMonday;

};

#endif // SCHEDULEWIDGET_H
