#include "studentinfowidget.h"
#include "ui_studentinfowidget.h"
#include<QSqlError>
#include<QMessageBox>
#include<QDebug>
#include<QInputDialog>
#include <QFileDialog>
#include <QBuffer>
#include<QDate>


StudentInfoWidget::StudentInfoWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StudentInfoWidget)
{
    ui->setupUi(this);


 ui->tableViewStudents->verticalHeader()->setVisible(false);

    ui->tableViewStudents->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableViewStudents->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableViewStudents->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    connect(ui->lineEditSearch, &QLineEdit::returnPressed,
            this, &StudentInfoWidget::on_btnSearch_clicked);
    model=new QSqlTableModel(this);
    model->setTable("studentInfo");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();//执行查询查看数据

    //设置表头
    model->setHeaderData(0,Qt::Horizontal,"学号");
    model->setHeaderData(1, Qt::Horizontal, "姓名");
    model->setHeaderData(2, Qt::Horizontal, "性别");
    model->setHeaderData(3, Qt::Horizontal, "生日");
    model->setHeaderData(4, Qt::Horizontal, "入学日期");
    model->setHeaderData(5, Qt::Horizontal, "学习目标");
    model->setHeaderData(6, Qt::Horizontal, "班级");
    model->setHeaderData(7, Qt::Horizontal, "照片");

    ui->tableViewStudents->setModel(model);
    //设置行为为选择整行
    ui->tableViewStudents->setSelectionBehavior(QAbstractItemView::SelectRows);
    //设置选择模式为单选
    ui->tableViewStudents->setSelectionMode(QAbstractItemView::SingleSelection);
    //设置表格无法编辑
    ui->tableViewStudents->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //表格交替颜色
    ui->tableViewStudents->setAlternatingRowColors(true);
    //设置最后一列自动拉伸填充满表格宽度
    ui->tableViewStudents->horizontalHeader()->setStretchLastSection(true);

    //隐藏照片列
    ui->tableViewStudents->setColumnHidden(7,true);

    refreshTable();
}

StudentInfoWidget::~StudentInfoWidget()
{
    delete ui;
}

void StudentInfoWidget::refreshTable()
{
    if(model)
    model->select();
}

//添加好友
void StudentInfoWidget::on_btnadd_clicked()
{

    if(!model)
        return;
    bool ok;
    //输入学号
    QString stuId=QInputDialog::getText(this,"新增学生","学生id:",QLineEdit::Normal,"",&ok);

    if (!ok || stuId.trimmed().isEmpty())
        return;
    //输入姓名
    QString stuName = QInputDialog::getText(
        this, "新增学生", "姓名：",
        QLineEdit::Normal, "", &ok);

    if (!ok || stuName.trimmed().isEmpty())
        return;
    //输入性别
    QString gender = QInputDialog::getItem(
        this, "新增学生", "性别：",
        QStringList() << "男" << "女",
        0,  // 默认选中第一个
        false,
        &ok);
    gender = gender.trimmed();
    if (!ok || gender.isEmpty())
        return;
    //生日
    // 4. 生日（简单用字符串，格式 yyyy-MM-dd）
    QString birthday = QInputDialog::getText(
        this, "新增学生", "生日（yyyy-MM-dd）：",
        QLineEdit::Normal, "", &ok);
    birthday = birthday.trimmed();
    if (!ok || birthday.isEmpty())
        return;
    //班级
    QString className = QInputDialog::getText(
        this, "新增学生", "班级：",
        QLineEdit::Normal, "", &ok);
    className = className.trimmed();
    if (!ok || className.isEmpty())
        return;

     QString joinDate = QDate::currentDate().toString("yyyy-MM-dd");
    //3插入到model中
    int row=model->rowCount();
    model->insertRow(row);
    model->setData(model->index(row,0),stuId);
    model->setData(model->index(row,1),stuName);
    model->setData(model->index(row, 2), gender);      // 性别
    model->setData(model->index(row, 3), birthday);    // 生日
    model->setData(model->index(row, 4), joinDate);    // 入学日期
    model->setData(model->index(row, 6), className);   // 班级（占用 progress 列）


    if(!model->submitAll()){
        QMessageBox::warning(this,"错误","保存失败"+model->lastError().text());
        model->revertAll();
        return;
    }
    refreshTable();
}

//删除行
void StudentInfoWidget::on_btndeleteLine_clicked()
{
    if (!model)
        return;
    QModelIndex index=ui->tableViewStudents->currentIndex();
    if(!index.isValid()){
        QMessageBox::information(this,"提示","请选中要删除的学生记录。");
        return;
    }
    int row=index.row();
    QString name=model->data(model->index(row, 1)).toString();
    auto ret=QMessageBox::question(this,"确定删除",QString("确定要删除该学生吗？\n\n学号：%1\n姓名：%2")
                                                           .arg(model->data(model->index(row, 0)).toString())
                                                           .arg(name),
                                     QMessageBox::Yes|QMessageBox::No
                                                             );
    if (ret != QMessageBox::Yes)
        return;
    model->removeRow(row);

    if (!model->submitAll()) {
        QMessageBox::warning(
            this,
            "错误",
            "删除失败：\n" + model->lastError().text()
            );
        model->revertAll();
        return;
    }
    refreshTable();


}

//删除项
void StudentInfoWidget::on_btndeleteItem_clicked()
{
    if (!model)
        return;

    QModelIndex index = ui->tableViewStudents->currentIndex();
    if (!index.isValid()) {
        QMessageBox::information(this, "提示", "请先选中要清空的单元格。");
        return;
    }
    int row = index.row();
    int col = index.column();

    auto ret = QMessageBox::question(
        this,
        "确认清空",
        QString("确定要清空这一格吗？\n\n列号：%1").arg(col),
        QMessageBox::Yes | QMessageBox::No
        );
    if (ret != QMessageBox::Yes)
        return;
    model->setData(model->index(row,col),QVariant());
    if (!model->submitAll()) {
        QMessageBox::warning(this, "错误", "清空失败：" + model->lastError().text());
        if (col == 7) {
            photoData.clear();
            ui->labelPhoto->setText("无照片");
            ui->labelPhoto->setPixmap(QPixmap());
        }
        model->revertAll();
        return;
    }

    refreshTable();


}

//更新图片
void StudentInfoWidget::on_btnChangePhoto_clicked()
{
    if(!model)
        return;
    QModelIndex index=ui->tableViewStudents->currentIndex();
    if(!index.isValid()){
        QMessageBox::information(this,"提示","请先选中一个学生");
        return;
    }
    int row=index.row();
    //2选择图片文件
    QString filePath=QFileDialog::getOpenFileName(this,"选择照片",QString(),"图片文件 (*.png *.jpg *.jpeg *.bmp)");

    if(filePath.isEmpty()){
        return;
    }
    //用qpixmap加载图片
    QPixmap pix(filePath);
    if(pix.isNull()){
        QMessageBox::warning(this,"错误","无法选中加载的图片文件");
        return;
    }
    //把图片转为qbytearray，存入数据库BLoB
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    if(!pix.save(&buffer,"PNG")){
        QMessageBox::warning(this,"错误","保存图片到内存失败");
        return;
    }
    //写入当前行的photo列里面
    model->setData(model->index(row,7),bytes);
    if (!model->submitAll()) {
        QMessageBox::warning(
            this,
            "错误",
            "更新照片失败：\n" + model->lastError().text()
            );
        model->revertAll();
        return;
    }
    //刷新右边预览
    photoData=bytes;
    QPixmap scaled = pix.scaled(
        ui->labelPhoto->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        );
    ui->labelPhoto->setPixmap(scaled);
    ui->labelPhoto->setText(QString());
}
//同步左边
void StudentInfoWidget::on_tableViewStudents_clicked(const QModelIndex &index)
{
    tableViewStudent(index);
}
//
void StudentInfoWidget::tableViewStudent(const QModelIndex &index)
{
    if(!index.isValid()||!model)
        return;
    int row= index.row();

    //studentInfo 列：0 id, 1 name, 2 gender, 3 birthday, 4 join_date, 5 study_goal, 6 progress, 7 photo
    ui->lineEditId->setText(model->data(model->index(row,0)).toString());
    ui->lineEditName->setText(model->data(model->index(row, 1)).toString());
    ui->lineEditGender->setText(model->data(model->index(row, 2)).toString());
    ui->lineEditClass->setText(model->data(model->index(row, 6)).toString());    // 临时显示 birthday
    ui->lineEditBriday->setText(model->data(model->index(row, 3)).toString());    // 临时显示 join_date

    QVariant photoVar =model->data(model->index(row, 7));
    photoData=photoVar.toByteArray();
    if(!photoData.isEmpty()){
        QPixmap pix;
        if(pix.loadFromData(photoData)){
            ui->labelPhoto->setPixmap(
                pix.scaled(ui->labelPhoto->size(),
                           //保持比例
                           Qt::KeepAspectRatio,
                           //缩放好看
                           Qt::SmoothTransformation
                    )
                );
            //清空文字
            ui->labelPhoto->setText(QString());
        }else{
            ui->labelPhoto->setText("图片损坏");
            //设置空图片
            ui->labelPhoto->setPixmap(QPixmap());
        }
    }else {
        ui->labelPhoto->setText("无照片");
        ui->labelPhoto->setPixmap(QPixmap());
    }

}


void StudentInfoWidget::on_btnSearch_clicked()
{
    if (!model)
        return;
      QString keyword = ui->lineEditSearch->text().trimmed();
       keyword.replace("'", "''");

      if (keyword.isEmpty()) {
          model->setFilter(QString());  // 清空过滤条件
          model->select();
          return;
      }
      QString filter = QString(
                           "(id   LIKE '%%1%' "
                           " OR name LIKE '%%1%')"
                           ).arg(keyword);


      model->setFilter(filter);
      if (!model->select()) {
          QMessageBox::warning(
              this,
              "查询失败",
              "搜索执行失败：\n" + model->lastError().text()
              );
          return;
      }

      if(model->rowCount()>0){
          QModelIndex first=model->index(0,0);
          ui->tableViewStudents->setCurrentIndex(first);
          tableViewStudent(first);
      }else {
          // 5. 没结果：清空右侧显示
          ui->lineEditId->clear();
          ui->lineEditName->clear();
          ui->lineEditGender->clear();
          ui->lineEditClass->clear();
          ui->lineEditBriday->clear();

          photoData.clear();
          ui->labelPhoto->setText("无照片");
          ui->labelPhoto->setPixmap(QPixmap());
      }
}

