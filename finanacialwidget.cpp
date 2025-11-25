#include "finanacialwidget.h"
#include "ui_finanacialwidget.h"
#include<QChartView>
#include<QSqlQuery>
#include<QDateTimeAxis>
#include<QPieSeries>
#include<QValueAxis>
#include<QMessageBox>
#include<QLineSeries>
#include<QSqlError>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QFormLayout>
#include<QDebug>
#include<QLegend>

FinanacialWidget::FinanacialWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FinanacialWidget)
{
    ui->setupUi(this);

    initUi();
    initConnections();
    loadStudents();
    loadRecords();  // 顺带刷新图表
}

FinanacialWidget::~FinanacialWidget()
{
    delete ui;
}

void FinanacialWidget::initUi()
{
    // 日期：默认最近一个月
    QDate today = QDate::currentDate();//获取系统当前时间
    ui->endDateEdit->setDate(today);
    ui->startDateEdit->setDate(today.addMonths(-1));
    //让 QDateEdit 在点击时 弹出日历选择器
    ui->startDateEdit->setCalendarPopup(true);
    ui->endDateEdit->setCalendarPopup(true);

    // 表格设置一点样式
    auto *table = ui->tableWidget;
    //开启交替行颜色（白/浅灰）
    table->setAlternatingRowColors(true);
    //设置“选中即整行”
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    //只允许选中一行
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    //禁止用户直接编辑表格内容
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //列宽自动平分整个表格宽度
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //隐藏第 0 列（ID）
    table->setColumnHidden(0, true);  // 隐藏 ID 列
}

void FinanacialWidget::initConnections()
{
    // 筛选条件变化时重新加载
    connect(ui->studentComboBox, &QComboBox::currentTextChanged,
            this, &FinanacialWidget::loadRecords);
    connect(ui->startDateEdit, &QDateEdit::dateChanged,
            this, &FinanacialWidget::loadRecords);
    connect(ui->endDateEdit, &QDateEdit::dateChanged,
            this, &FinanacialWidget::loadRecords);


}

//把数据库里的学生名单加载到下拉框中，让财务记录可以按“学生”进行筛选。
void FinanacialWidget::loadStudents()
{
    //初始化
    ui->studentComboBox->clear();
    ui->studentComboBox->addItem("所有学生", -1);

    QSqlQuery query("SELECT id, name FROM studentInfo ORDER BY name");
    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        ui->studentComboBox->addItem(name, id);
    }
}
//筛选数据库的财务记录，然后把结果展示在表格上，同时统计两类数据供图表使用。
void FinanacialWidget::loadRecords()
{
        //初始化，因为每次都是重新加载
        auto *table = ui->tableWidget;
        table->setRowCount(0);

        //
        payTypeSum.clear();
        dateSum.clear();

        //拿id，起始和结束日期
        int studentId = ui->studentComboBox->currentData().toInt();
        QDate start   = ui->startDateEdit->date();
        QDate end     = ui->endDateEdit->date();

        //根据学生和时间范围，查出所有满足条件的财务记录
        QString sql =
            "SELECT fr.id, s.name, fr.payment_date, fr.amount, fr.payment_type, fr.notes "
            "FROM financialRecords fr "
            "JOIN studentInfo s ON fr.student_id = s.id "
            "WHERE fr.payment_date BETWEEN :start AND :end";

        //只有当用户选了某个具体学生时，才按学生筛选。
        if (studentId != -1) {
            sql += " AND fr.student_id = :studentId";
        }
        sql += " ORDER BY fr.payment_date ASC";

        //执行
        QSqlQuery query;
        query.prepare(sql);
        query.bindValue(":start", start.toString("yyyy-MM-dd"));
        query.bindValue(":end",   end.toString("yyyy-MM-dd"));
        if (studentId != -1) {
            query.bindValue(":studentId", studentId);
        }

        if (!query.exec()) {
            QMessageBox::warning(this, "错误",
                                 "加载财务记录失败：\n" + query.lastError().text());
            return;
        }

        //将查询结果塞入到表格
        int row = 0;
        while (query.next()) {
            table->insertRow(row);

            int id          = query.value(0).toInt();
            QString name    = query.value(1).toString();
            QString dateStr = query.value(2).toString();
            double amount   = query.value(3).toDouble();
            QString type    = query.value(4).toString();
            QString notes   = query.value(5).toString();

            //封装一个lambda
            auto setItem = [&](int col, const QString &text, Qt::Alignment align = Qt::AlignCenter){
                auto *item = new QTableWidgetItem(text);//创建表格条目
                item->setTextAlignment(align);//设置对齐方式
                table->setItem(row, col, item);//插入到表格
            };

            setItem(0, QString::number(id));
            setItem(1, name, Qt::AlignLeft | Qt::AlignVCenter);
            setItem(2, dateStr);
            setItem(3, QString::number(amount, 'f', 2),
                    Qt::AlignRight | Qt::AlignVCenter);
            setItem(4, type);
            setItem(5, notes, Qt::AlignLeft | Qt::AlignVCenter);

            // 边遍历边统计图表数据
            QDate d = QDate::fromString(dateStr, "yyyy-MM-dd");//转为QDate
            payTypeSum[type] += amount;//饼图支付方式加入金额
            dateSum[d]       += amount;//折线图日期加入金额

            ++row;
        }

        refreshCharts();

}

//有新学生的时候即时更新下拉框
void FinanacialWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    int oldId = ui->studentComboBox->currentData().toInt();

    loadStudents();   // ⭐ 每次显示模块时更新学生列表

    // 尝试恢复之前的选中项
    int index = ui->studentComboBox->findData(oldId);
    if (index >= 0)
        ui->studentComboBox->setCurrentIndex(index);

    // 更新表格和图表，让显示的数据也同步刷新
    loadRecords();
}

//刷新饼图和折线图，把新图标放在界面上
void FinanacialWidget::refreshCharts()
{
    // ----- 饼图：支付方式分布 -----
    QChart *pie = new QChart;
    pie->setTitle("支付类型分布");

    //保存数据 ，饼状保存数据
    QPieSeries *series = new QPieSeries;
    //遍历饼状数据
    for (auto it = payTypeSum.begin(); it != payTypeSum.end(); ++it) {
        //跳过0
        if (it.value() <= 0)
            continue;
        //标签
        QString label = QString("%1 %2元").arg(it.key()).arg(it.value());
        //给他名字和占比
        series->append(label, it.value());
    }
    //画布里面加数据
    pie->addSeries(series);
    //图例位置在底部
    pie->legend()->setAlignment(Qt::AlignBottom);


    //每次都更新图
    if (ui->pieChartView->chart())
        delete ui->pieChartView->chart();
    ui->pieChartView->setChart(pie);
    pie->legend()->setFont(QFont("Microsoft YaHei", 7)); // 字体变小一点




    // ======== 折线图部分（你原来已有，下面保留结构） ========

    // ----- 折线图：日期-金额 -----
    QChart *line = new QChart;
    line->setTitle("收支趋势");

    //折线图数据系列
    QLineSeries *lineSeries = new QLineSeries;

    // 构造日期序列
    QList<QDate> dates = dateSum.keys();
    std::sort(dates.begin(), dates.end());

    if (!dates.isEmpty()) {
        for (const QDate &d : dates) {
            QDateTime dt(d.startOfDay());
            lineSeries->append(dt.toMSecsSinceEpoch(), dateSum.value(d));
        }
    }

    //折线加入到图表中
    line->addSeries(lineSeries);

    // X 轴 日期轴
    auto *axisX = new QDateTimeAxis;
    axisX->setFormat("yyyy-MM-dd");
    axisX->setTitleText("日期");

    // 自动范围：最小日期、最大日期
    if (!dates.isEmpty()) {
        QDateTime minDT(dates.first().startOfDay());
        QDateTime maxDT(dates.last().startOfDay());

        axisX->setMin(minDT);
        axisX->setMax(maxDT);
    }

    // Y 轴
    auto *axisY = new QValueAxis;
    axisY->setTitleText("金额（元）");

    // 自动 Y 范围
    double maxValue = 0;
    for (auto v : dateSum) {
        if (v > maxValue) maxValue = v;
    }
    axisY->setRange(0, maxValue * 1.2); // 给图表留点上下边距

    //把轴挂到图表
    line->addAxis(axisX, Qt::AlignBottom);
    line->addAxis(axisY, Qt::AlignLeft);
    //把折线绑定到这些轴上
    lineSeries->attachAxis(axisX);
    lineSeries->attachAxis(axisY);

    //删掉
    if (ui->chartView->chart())
        delete ui->chartView->chart();
    ui->chartView->setChart(line);
}

void FinanacialWidget::on_addButton_clicked()
{
    // 弹一个小对话框：学生 + 日期 + 金额 + 类型 + 备注
    QDialog dialog(this);
    dialog.setWindowTitle("添加缴费记录");
    QFormLayout form(&dialog);

    // 学生下拉
    QComboBox *studentNameComboBox = new QComboBox(&dialog);
    {
        QSqlQuery q("SELECT id, name FROM studentInfo ORDER BY name");
        while (q.next()) {
            QString id   = q.value(0).toString();
            QString name = q.value(1).toString();
            studentNameComboBox->addItem(name, id);  // data 里放 id
        }
    }
    form.addRow("学生：", studentNameComboBox);

    // 日期
    QDateEdit *dateEdit = new QDateEdit(QDate::currentDate(), &dialog);
    dateEdit->setCalendarPopup(true);
    form.addRow("缴费日期：", dateEdit);

    // 金额
    QLineEdit *amountEdit = new QLineEdit(&dialog);
    amountEdit->setPlaceholderText("例如：1000.00");
    form.addRow("金额：", amountEdit);

    // 支付类型
    QComboBox *typeCombo = new QComboBox(&dialog);
    typeCombo->addItems({"微信", "支付宝", "现金", "银行卡", "其他"});
    form.addRow("支付类型：", typeCombo);

    // 备注
    QLineEdit *remarkEdit = new QLineEdit(&dialog);
    form.addRow("备注：", remarkEdit);

    // 按钮
    QDialogButtonBox buttonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;



    // 校验
    QString studentId = studentNameComboBox->currentData().toString();
    QDate   payDate   = dateEdit->date();
    QString amountStr = amountEdit->text().trimmed();
    QString type      = typeCombo->currentText().trimmed();
    QString remark    = remarkEdit->text().trimmed();

    //检查必填项
    if (studentId.isEmpty() || amountStr.isEmpty() || type.isEmpty()) {
        QMessageBox::warning(this, "错误", "学生、金额、支付类型不能为空。");
        return;
    }

    //金额要是数字且大于0
    bool ok = false;
    double amount = amountStr.toDouble(&ok);
    if (!ok || amount <= 0) {
        QMessageBox::warning(this, "错误", "金额格式不正确。");
        return;
    }

    // 插入数据库
    QSqlQuery query;
    query.prepare(
        "INSERT INTO financialRecords(student_id, payment_date, amount, payment_type, notes) "
        "VALUES(:student_id, :payment_date, :amount, :payment_type, :notes)");
    query.bindValue(":student_id",   studentId);
    query.bindValue(":payment_date", payDate.toString("yyyy-MM-dd"));
    query.bindValue(":amount",       amount);
    query.bindValue(":payment_type", type);
    query.bindValue(":notes",        remark);

    if (!query.exec()) {
        qDebug() << "添加记录失败:" << query.lastError();
        QMessageBox::warning(this, "错误", "添加记录失败：\n" + query.lastError().text());
        return;
    }

    // 刷新表格 + 图表
    loadRecords();
}


void FinanacialWidget::on_deleteButton_clicked()
{
    QTableWidget *table = ui->tableWidget;
    int currentRow = table->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "警告", "请选择要删除的记录！");
        return;
    }

    int id = table->item(currentRow, 0)->text().toInt();

    auto ret = QMessageBox::question(
        this,
        "确认删除",
        "确定要删除该记录吗？",
        QMessageBox::Yes | QMessageBox::No
        );
    if (ret != QMessageBox::Yes)
        return;

    QSqlQuery query;
    query.prepare("DELETE FROM financialRecords WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "删除记录失败:" << query.lastError();
        QMessageBox::warning(this, "错误", "删除记录失败：\n" + query.lastError().text());
        return;
    }

    loadRecords();
}


void FinanacialWidget::on_editButton_clicked()
{
    //从表格中拿到这条记录
    auto *table = ui->tableWidget;
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选择一条要编辑的记录。");
        return;
    }

    QTableWidgetItem *idItem    = table->item(row, 0);
    QTableWidgetItem *nameItem  = table->item(row, 1);
    QTableWidgetItem *dateItem  = table->item(row, 2);
    QTableWidgetItem *amtItem   = table->item(row, 3);
    QTableWidgetItem *typeItem  = table->item(row, 4);
    QTableWidgetItem *notesItem = table->item(row, 5);

    if (!idItem || !nameItem || !dateItem || !amtItem || !typeItem || !notesItem) {
        QMessageBox::warning(this, "错误", "选中行数据不完整，无法编辑。");
        return;
    }

    int     id          = idItem->text().toInt();
    QString studentName = nameItem->text();
    QString dateStr     = dateItem->text();
    QString amountStr   = amtItem->text();
    QString type        = typeItem->text();
    QString notes       = notesItem->text();

    // 弹出编辑对话框
    QDialog dialog(this);
    dialog.setWindowTitle("编辑缴费记录");
    QFormLayout form(&dialog);

    // 学生下拉：同添加，默认选中原来的学生
    QComboBox *studentNameComboBox = new QComboBox(&dialog);
    {
        QSqlQuery q("SELECT id, name FROM studentInfo ORDER BY name");
        int currentIndex = -1;
        int idx = 0;
        while (q.next()) {
            QString stuId   = q.value(0).toString();
            QString stuName = q.value(1).toString();
            studentNameComboBox->addItem(stuName, stuId);
            if (stuName == studentName) {
                currentIndex = idx;
            }
            ++idx;
        }
        if (currentIndex >= 0)
            studentNameComboBox->setCurrentIndex(currentIndex);
    }
    form.addRow("学生：", studentNameComboBox);

    // 日期
    QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
    if (!date.isValid())
        date = QDate::currentDate();

    QDateEdit *dateEdit = new QDateEdit(date, &dialog);
    dateEdit->setCalendarPopup(true);
    form.addRow("缴费日期：", dateEdit);

    // 金额
    QLineEdit *amountEdit = new QLineEdit(&dialog);
    amountEdit->setText(amountStr);
    form.addRow("金额：", amountEdit);

    // 支付类型
    QComboBox *typeCombo = new QComboBox(&dialog);
    typeCombo->addItems({"微信", "支付宝", "现金", "银行卡", "其他"});
    int typeIndex = typeCombo->findText(type);
    if (typeIndex >= 0)
        typeCombo->setCurrentIndex(typeIndex);
    form.addRow("支付类型：", typeCombo);

    // 备注
    QLineEdit *remarkEdit = new QLineEdit(&dialog);
    remarkEdit->setText(notes);
    form.addRow("备注：", remarkEdit);

    // 按钮
    QDialogButtonBox buttonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    // --- 校验 ---
    QString studentIdNew = studentNameComboBox->currentData().toString();
    QDate   payDateNew   = dateEdit->date();
    QString amountStrNew = amountEdit->text().trimmed();
    QString typeNew      = typeCombo->currentText().trimmed();
    QString notesNew     = remarkEdit->text().trimmed();

    if (studentIdNew.isEmpty() || amountStrNew.isEmpty() || typeNew.isEmpty()) {
        QMessageBox::warning(this, "错误", "学生、金额、支付类型不能为空。");
        return;
    }

    bool ok = false;
    double amountNew = amountStrNew.toDouble(&ok);
    if (!ok || amountNew <= 0) {
        QMessageBox::warning(this, "错误", "金额格式不正确。");
        return;
    }

    // --- 更新数据库 ---
    QSqlQuery query;
    query.prepare(
        "UPDATE financialRecords "
        "SET student_id = :student_id, "
        "    payment_date = :payment_date, "
        "    amount = :amount, "
        "    payment_type = :payment_type, "
        "    notes = :notes "
        "WHERE id = :id");
    query.bindValue(":student_id",   studentIdNew);
    query.bindValue(":payment_date", payDateNew.toString("yyyy-MM-dd"));
    query.bindValue(":amount",       amountNew);
    query.bindValue(":payment_type", typeNew);
    query.bindValue(":notes",        notesNew);
    query.bindValue(":id",           id);

    if (!query.exec()) {
        QMessageBox::warning(this, "错误",
                             "更新记录失败：\n" + query.lastError().text());
        return;
    }

    // 刷新表格 + 图表（如果日期被改到筛选范围外，这条记录会自然消失，这是合理行为）
    loadRecords();
}

