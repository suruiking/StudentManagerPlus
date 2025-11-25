#include "honorwallwidget.h"
#include "ui_honorwallwidget.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QFileDialog>
#include <QBuffer>
#include <QInputDialog>
#include <QListWidgetItem>

HonorWallWidget::HonorWallWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HonorWallWidget)
{
    ui->setupUi(this);
    initUi();
    initConnections();
    loadHonors();
}

HonorWallWidget::~HonorWallWidget()
{
    delete ui;
}

//初始化ui
void HonorWallWidget::initUi()
{
    ui->previewLabel->setText("请选择左侧的荣誉照片");
    ui->previewLabel->setAlignment(Qt::AlignCenter);
    ui->previewLabel->setMinimumSize(300, 200);

    ui->descLabel->setText(QString());
    ui->dateLabel->setText(QString());

    // 列表显示成图标模式（缩略图 + 文本）
    ui->honorListWidget->setViewMode(QListView::IconMode);
    ui->honorListWidget->setIconSize(QSize(120, 90));
    ui->honorListWidget->setResizeMode(QListView::Adjust);
    ui->honorListWidget->setMovement(QListView::Static);
}

void HonorWallWidget::initConnections()
{

}

//从数据库拉记录出来，缓存到内存，然后左边是缩略图，选中后刷新右边预览
void HonorWallWidget::loadHonors()
{
    ui->honorListWidget->clear();
    items_.clear();

    //sql查询
    QSqlQuery query;
    query.prepare("SELECT id, image_data, description, added_date "
                  "FROM honorWall "
                  "ORDER BY added_date DESC, id DESC");
    if (!query.exec()) {
        QMessageBox::warning(this, "错误",
                             "加载荣誉墙失败：\n" + query.lastError().text());
        return;
    }

    //
    while (query.next()) {
        HonorItem item;
        item.id          = query.value(0).toInt();
        QByteArray bytes = query.value(1).toByteArray();
        item.description = query.value(2).toString();
        item.addedDate   = QDate::fromString(query.value(3).toString(), "yyyy-MM-dd");

        item.image.loadFromData(bytes);

        items_.push_back(item);

        // 列表项：缩略图 + 简短文字
        QIcon icon(item.image.scaled(120, 90, Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation));

        //防止过长
        QString text = item.description;
        if (text.length() > 10)
            text = text.left(10) + "...";

        //塞进列表
        auto *wItem = new QListWidgetItem(icon, text);
        wItem->setData(Qt::UserRole, item.id);   // 备用：存一下 id
        ui->honorListWidget->addItem(wItem);
    }

    //加载完自动选中第一条
    if (!items_.isEmpty())
        ui->honorListWidget->setCurrentRow(0);
    else
        showCurrentHonor();  // 清空预览
}

//左边选中，右边展示
void HonorWallWidget::showCurrentHonor()
{
    int row = ui->honorListWidget->currentRow();
    //不合法，就清空
    if (row < 0 || row >= items_.size()) {
        ui->previewLabel->setPixmap(QPixmap());
        ui->previewLabel->setText("暂无荣誉照片");
        ui->descLabel->setText(QString());
        ui->dateLabel->setText(QString());
        return;
    }
//拿到这条荣誉的完整数据
    const HonorItem &item = items_.at(row);

    // 缩放到预览区域大小，保持比例
    QPixmap scaled = item.image.scaled(ui->previewLabel->size(),
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
    ui->previewLabel->setPixmap(scaled);
    ui->previewLabel->setText(QString());

    ui->descLabel->setText(item.description);
    ui->dateLabel->setText("添加日期：" + item.addedDate.toString("yyyy-MM-dd"));
}

void HonorWallWidget::on_addButton_clicked()
{
    // 1. 选择图片
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择荣誉照片",
        QString(),
        "Images (*.png *.jpg *.jpeg *.bmp)"
        );
    if (fileName.isEmpty())
        return;

    //错误判断
    QPixmap pix(fileName);
    if (pix.isNull()) {
        QMessageBox::warning(this, "错误", "无法加载所选图片。");
        return;
    }

    // 2. 输入描述
    bool ok = false;
    QString desc = QInputDialog::getText(
        this,
        "输入说明",
        "请输入荣誉说明：",
        QLineEdit::Normal,
        QString(),
        &ok
        );
    if (!ok)   // 取消
        return;

    // 3. 转成字节数组
    //qbuffer是文件写入器
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    //通过buffer写入到bytes里面
    pix.save(&buffer, "PNG");

    QDate today = QDate::currentDate();

    //插入到数据库
    QSqlQuery query;
    query.prepare("INSERT INTO honorWall(image_data, description, added_date) "
                  "VALUES(:img, :desc, :date)");
    query.bindValue(":img",  bytes);
    query.bindValue(":desc", desc);
    query.bindValue(":date", today.toString("yyyy-MM-dd"));

    if (!query.exec()) {
        QMessageBox::warning(this, "错误",
                             "添加荣誉失败：\n" + query.lastError().text());
        return;
    }

    // 4. 从数据库重载
    loadHonors();
}


void HonorWallWidget::on_deleteButton_clicked()
{
    int row = ui->honorListWidget->currentRow();
    if (row < 0 || row >= items_.size()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的荣誉照片。");
        return;
    }

    int id = items_.at(row).id;

    auto ret = QMessageBox::question(
        this,
        "确认删除",
        "确定要删除这条荣誉记录吗？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );
    if (ret != QMessageBox::Yes)
        return;

    QSqlQuery query;
    query.prepare("DELETE FROM honorWall WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        QMessageBox::warning(this, "错误",
                             "删除失败：\n" + query.lastError().text());
        return;
    }

    loadHonors();
}

void HonorWallWidget::on_honorListWidget_currentRowChanged(int row)
{
      showCurrentHonor();
}

