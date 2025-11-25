#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QButtonGroup>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/res/tubiao.webp"));
    QButtonGroup*btnGp=new QButtonGroup(this);
    btnGp->addButton(ui->btnxueyuan,0);
    btnGp->addButton(ui->btnkecheng,1);
    btnGp->addButton(ui->btncaiwu,2);
    btnGp->addButton(ui->btnrongyu,3);
    btnGp->addButton(ui->btnshezhi,4);
    connect(btnGp,&QButtonGroup::idClicked,ui->stackedWidget,&QStackedWidget::setCurrentIndex);

    ui->stackedWidget->setCurrentWidget(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}
