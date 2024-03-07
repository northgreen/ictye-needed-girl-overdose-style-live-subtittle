#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <server.h>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_sandButton_clicked();

    void on_lineEdit_textChanged(const QString &arg1);

    void on_seriverSartButturn_clicked();

    void on_Widget_destroyed();

    void new_log(const QString &qrg1);

    void on_seriverStopButturn_clicked();

    void on_lineEdit_editingFinished();

    void on_lineEdit_2_editingFinished();

private:
    Ui::Widget *ui;
    ServerThread *serverthread;

};
#endif // WIDGET_H
