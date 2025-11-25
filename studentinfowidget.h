#ifndef STUDENTINFOWIDGET_H
#define STUDENTINFOWIDGET_H

#include <QWidget>
#include<QSqlTableModel>

namespace Ui {
class StudentInfoWidget;
}

class StudentInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StudentInfoWidget(QWidget *parent = nullptr);
    ~StudentInfoWidget();
    void refreshTable();//刷新表格


private slots:
    void on_btnadd_clicked();

    void on_btndeleteLine_clicked();

    void on_btndeleteItem_clicked();

    void on_btnChangePhoto_clicked();

    void  tableViewStudent(const QModelIndex &index);

    void on_tableViewStudents_clicked(const QModelIndex &index);



    void on_btnSearch_clicked();

private:
    Ui::StudentInfoWidget *ui;
    QSqlTableModel*model;
    QByteArray photoData;
};

#endif // STUDENTINFOWIDGET_H
