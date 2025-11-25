#include "schedulewidget.h"
#include "ui_schedulewidget.h"
#include<QTableWidget>
#include<QHeaderView>
#include<QMessageBox>
#include<QInputDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

ScheduleWidget::ScheduleWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScheduleWidget)
{
    ui->setupUi(this);

    QDate today=QDate::currentDate();//得到今天的日期
    int dow=today.dayOfWeek();//获得日期的周几
    currentMonday=today.addDays(1-dow);//变成周一

    initUi();
    initWeekInfo();

    refreshTable();
}

ScheduleWidget::~ScheduleWidget()
{
    delete ui;
}

//初始化ui
void ScheduleWidget::initUi()
{
    QTableWidget*table=ui->tableWidgetSchedule;
    table->setRowCount(7);
    table->setColumnCount(6);

    QStringList colHeaders;
    colHeaders << "上午1" << "上午2" << "下午1" << "下午2" << "晚上1" << "晚上2";
    table->setHorizontalHeaderLabels(colHeaders);
    QStringList rowHeaders;
    rowHeaders << "周一" << "周二" << "周三" << "周四" << "周五" << "周六" << "周日";
    //为垂直表头设置标签文本
    table->setVerticalHeaderLabels(rowHeaders);

    //设置表头的
    //列标题设置宽度120
    table->horizontalHeader()->setDefaultSectionSize(120);
    //最后一列自动拉伸
    table->horizontalHeader()->setStretchLastSection(true);
    //垂直表头默认高度60
    table->verticalHeader()->setDefaultSectionSize(60);

    //选择模式单选
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    //选中单个单元格
    table->setSelectionBehavior(QAbstractItemView::SelectItems);
    //禁止编辑触发机制
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //交替颜色
    table->setAlternatingRowColors(true);



}
//初始化顶部两个下拉框，年份和周数
void ScheduleWidget::initWeekInfo()
{
    int year = currentMonday.year();
    ui->comboBoxYear->clear();
    for (int y = year - 1; y <= year + 1; ++y) {
        ui->comboBoxYear->addItem(QString::number(y));
    }


    ui->comboBoxWeek->clear();
    // 建 1~53 周
    for (int w = 1; w <= 53; ++w) {
        ui->comboBoxWeek->addItem(QString("第 %1 周").arg(w), w);
    }

    updateWeekLabels();

}
//更新本周日期范围
void ScheduleWidget::updateWeekLabels()
{
    QDate monday = currentMonday;
    QDate sunday = currentMonday.addDays(6);

    int weekNum = monday.weekNumber();
    int isoYear = monday.addDays(3).year();   // ISO：同一周的周四属于哪一年

    // 同步年份下拉框（屏蔽信号，防止触发槽函数）
    {
        //QSignalBlocker blocker(ui->comboBoxYear);
        int yearIndex = ui->comboBoxYear->findText(QString::number(isoYear));
        if (yearIndex >= 0)
            ui->comboBoxYear->setCurrentIndex(yearIndex);
    }

    // 同步周数下拉框
    {
       // QSignalBlocker blocker(ui->comboBoxWeek);
        int weekIndex = ui->comboBoxWeek->findData(weekNum);
        if (weekIndex >= 0)
            ui->comboBoxWeek->setCurrentIndex(weekIndex);
    }

    ui->labelWeekRange->setText(
        QString("%1 到 %2")
            .arg(monday.toString("yyyy-MM-dd"))
            .arg(sunday.toString("yyyy-MM-dd")));
}

//先刷新表头 → 清空格子 → 查本周课程 → 映射到表格 → 更新顶部周信息。
void ScheduleWidget::refreshTable()
{
    //拿表格指针
    QTableWidget*table=ui->tableWidgetSchedule;
    //刷新行头
    QStringList rowHeaders;
    //遍历7天，分别设置名字
    for(int i=0;i<7;i++){
        QDate d=currentMonday.addDays(i);
        QString weekName;

        switch(d.dayOfWeek()){
        case 1: weekName = "星期一"; break;
        case 2: weekName = "星期二"; break;
        case 3: weekName = "星期三"; break;
        case 4: weekName = "星期四"; break;
        case 5: weekName = "星期五"; break;
        case 6: weekName = "星期六"; break;
        case 7: weekName = "星期日"; break;
        }
        rowHeaders << QString("%1\n%2")
                          .arg(weekName)
                          .arg(d.toString("MM/dd"));

    }
    //设置行头
    table->setVerticalHeaderLabels(rowHeaders);

    // 先清空内容（后面再让数据库填）
    table->clearContents();

    // 3. 从数据库加载这一周的课程
    QSqlQuery query;
    //获得周一和周日
    QString mondayStr = currentMonday.toString("yyyy-MM-dd");
    QString sundayStr = currentMonday.addDays(6).toString("yyyy-MM-dd");

    //从星期一到星期日之间的所有课程记录
    query.prepare("SELECT date, time, course_name FROM schedule "
                  "WHERE date >= :monday AND date <= :sunday");
    query.bindValue(":monday", mondayStr);
    query.bindValue(":sunday", sundayStr);

    //查到然后做错误判断
    if (!query.exec()) {
        qDebug() << "加载课程失败:" << query.lastError();
        QMessageBox::warning(this, "错误",
                             "加载课程失败：\n" + query.lastError().text());
        updateWeekLabels();
        return;
    }

    //然后遍历结果，把数据库记录映射到表格坐标
    while (query.next()) {
        //行就是日期，时间，课程，一次遍历一行
        QString dateStr    = query.value(0).toString();
        QString timeKey    = query.value(1).toString();
        QString courseName = query.value(2).toString();

        QDate d = QDate::fromString(dateStr, "yyyy-MM-dd");
        if (!d.isValid())
            continue;

        //这是获得行坐标
        int row = currentMonday.daysTo(d);  // date - monday
        //错误判断
        if (row < 0 || row >= 7)
            continue;

        //这是获得列坐标
        int col = -1;
        if      (timeKey == "上午1") col = 0;
        else if (timeKey == "上午2") col = 1;
        else if (timeKey == "下午1") col = 2;
        else if (timeKey == "下午2") col = 3;
        else if (timeKey == "晚上1") col = 4;
        else if (timeKey == "晚上2") col = 5;

        if (col < 0 || col >= 6)
            continue;

        QTableWidgetItem *item = new QTableWidgetItem(courseName);
        item->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, col, item);
    }

    updateWeekLabels();
}

//用当前下拉框的年和周重新计算周一
void ScheduleWidget::updateCurrentMondayFromCombo()
{
    int year    = ui->comboBoxYear->currentText().toInt();
    int weekNum = ui->comboBoxWeek->currentData().toInt();  // 用 userData 存周号

    if (year <= 0 || weekNum <= 0)
        return;

    // == mondayOfIsoWeek 合并版 ==
    // ISO 周1 一定包含 1 月 4 日
    QDate base(year, 1, 4);
    int delta = base.dayOfWeek() - 1;        // dayOfWeek: 1=周一, 7=周日
    QDate mondayWeek1 = base.addDays(-delta);//算出第一周的周一

    currentMonday = mondayWeek1.addDays((weekNum - 1) * 7);//算出第几周的星期一
}
void ScheduleWidget::on_comboBoxWeek_currentIndexChanged(int index)
{
    if (index < 0)
        return;

    updateCurrentMondayFromCombo();

    refreshTable();       // 重新加载本周课程
    updateWeekLabels();   // 同步 label + 下拉框（其实下拉框已经选好，只是更新日期范围）
}


void ScheduleWidget::on_comboBoxYear_currentIndexChanged(int index)
{
    if (index < 0)
        return;

    updateCurrentMondayFromCombo();  // 根据 年 + 周 算出新的 currentMonday

    refreshTable();       // 重刷这一周的课程（可选，你要不要一起刷新）
    updateWeekLabels();   // ⭐ 这里更新的就是你说的最右边标签
}




//你点表格的某个格子（row, column），
//这俩函数把它翻译成数据库的 (date, timeKey)，

//行号 → 日期。
QDate ScheduleWidget::dateForRow(int row) const
{
     return currentMonday.addDays(row);
}

//第 column 列对应的时间段 key 是什么。
QString ScheduleWidget::timeSlotKeyForColumn(int column) const
{
    switch (column) {
    case 0: return "上午1";
    case 1: return "上午2";
    case 2: return "下午1";
    case 3: return "下午2";
    case 4: return "晚上1";
    case 5: return "晚上2";
    default: return QString();
    }
}

//上一周
void ScheduleWidget::on_btnPrevWeek_clicked()
{
    currentMonday=currentMonday.addDays(-7);
    refreshTable();
}

//下一周
void ScheduleWidget::on_btnNextWeek_clicked()
{
    currentMonday = currentMonday.addDays(7);
    refreshTable();
}

//添加课程
void ScheduleWidget::on_btnAddCourse_clicked()
{
    QTableWidget *table = ui->tableWidgetSchedule;
    QModelIndex idx = table->currentIndex();

    if (!idx.isValid()) {
        QMessageBox::information(this, "提示", "请先选中一个时间格子再添加课程。");
        return;
    }

    int row = idx.row();
    int col = idx.column();

    // 当前日期 & 时间段 key
    QDate date = dateForRow(row);
    QString timeKey = timeSlotKeyForColumn(col);
    if (!date.isValid() || timeKey.isEmpty())
        return;

    // 原课程名作为默认输入
    QString oldName;
    if (auto *item = table->item(row, col)) {
        oldName = item->text();
    }
    //输入新课程名
    bool ok = false;
    QString newName = QInputDialog::getText(
        this,
        "编辑课程",
        "课程名称：",
        QLineEdit::Normal,
        oldName,
        &ok
        );
    if (!ok) {
        return;
    }
    newName = newName.trimmed();
    //把QDate(2025, 01, 08)转字符串
    QString dateStr = date.toString("yyyy-MM-dd");

    QSqlQuery query;
    // 1. 先删掉原记录（如果有）
    query.prepare("DELETE FROM schedule WHERE date = :date AND time = :time");
    query.bindValue(":date", dateStr);
    query.bindValue(":time", timeKey);
    if (!query.exec()) {
        QMessageBox::warning(this, "错误",
                             "删除原课程失败：\n" + query.lastError().text());
        return;
    }

    // 2. 输入不为空，则插入新记录；为空就只当作删除
    if (!newName.isEmpty()) {
        query.prepare("INSERT INTO schedule(date, time, course_name) "
                      "VALUES(:date, :time, :course)");
        query.bindValue(":date",   dateStr);
        query.bindValue(":time",   timeKey);
        query.bindValue(":course", newName);

        if (!query.exec()) {
            QMessageBox::warning(this, "错误",
                                 "保存课程失败：\n" + query.lastError().text());
            return;
        }
    }

    // 3. 重新刷新课表
    refreshTable();

}

//删除课程
void ScheduleWidget::on_btnRemoveCourse_clicked()
{
    QTableWidget *table = ui->tableWidgetSchedule;
    QModelIndex idx = table->currentIndex();

    if (!idx.isValid()) {
        QMessageBox::information(this, "提示", "请先选中要删除的课程格子。");
        return;
    }

    int row = idx.row();
    int col = idx.column();

    QTableWidgetItem *item = table->item(row, col);
    if (!item || item->text().trimmed().isEmpty()) {
        QMessageBox::information(this, "提示", "这个时间格子本来就没有课程。");
        return;
    }

    QString courseName = item->text();

    QDate date = dateForRow(row);
    QString timeKey = timeSlotKeyForColumn(col);
    if (!date.isValid() || timeKey.isEmpty())
        return;

    auto ret = QMessageBox::question(
        this,
        "确认删除",
        QString("确定要删除该课程吗？\n\n日期：%1\n时间：%2\n课程：%3")
            .arg(date.toString("yyyy-MM-dd"))
            .arg(timeKey)
            .arg(courseName),
        QMessageBox::Yes | QMessageBox::No
        );
    if (ret != QMessageBox::Yes)
        return;

    QString dateStr = date.toString("yyyy-MM-dd");

    QSqlQuery query;
    query.prepare("DELETE FROM schedule WHERE date = :date AND time = :time");
    query.bindValue(":date", dateStr);
    query.bindValue(":time", timeKey);

    if (!query.exec()) {
        QMessageBox::warning(this, "错误",
                             "删除课程失败：\n" + query.lastError().text());
        return;
    }

    refreshTable();

}

