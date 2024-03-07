#include "widget.h"
#include "./ui_widget.h"
#include <QMessageBox>
#include "httplib.h"
#include <QThread>
#include <server.h>
#include <QRegExp>
#include <QRegExpValidator>
#include <QVBoxLayout>


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    ,serverthread(new ServerThread)
{
    ui->setupUi(this);
    // 启动信号管理
    connect(this->serverthread,&ServerThread::serverStopped,ui->seriverSartButturn,&QPushButton::setEnabled);
    connect(this->serverthread,&ServerThread::serverStarted,ui->seriverStopButturn,&QPushButton::setEnabled);
    connect(this->serverthread,&ServerThread::serverStarted,ui->seriverSartButturn,&QPushButton::setDisabled);
    connect(this->serverthread,&ServerThread::serverStopped,ui->seriverStopButturn,&QPushButton::setDisabled);

    // 日志
    connect(this->serverthread,&ServerThread::serverLog,this,&Widget::new_log);

    // 输入正确性验证
    QRegExp portRegex("^(0|([1-9][0-9]{0,3}))$"); // 正则表达式匹配端口号范围 0 到 65535
    QRegExpValidator *portValidator = new QRegExpValidator(portRegex, ui->lineEdit_2);
    ui->lineEdit_2->setValidator(portValidator);
}

Widget::~Widget()
{
    serverthread->stop();
    serverthread->quit();
    serverthread->wait();
    delete  serverthread;
    delete ui;
}


// 發送消息
void Widget::on_sandButton_clicked()
{
    this->serverthread->sendsubtittle(ui->lineEdit->text());
    ui->lineEdit->setText("");
}


// 文字合法性檢查
void Widget::on_lineEdit_textChanged(const QString &arg1)
{
    if(arg1 == ""){
        ui->sandButton->setEnabled(0);
    }else{
        ui->sandButton->setEnabled(1);
    }
}


// http服務啓動
void Widget::on_seriverSartButturn_clicked()
{
    try {
        // 启动 HTTP 服务器

        serverthread->start();
    } catch (const std::exception& e) {
        // 显示启动失败的错误信息
        QMessageBox::critical(nullptr, "Error", QString("Failed to start server: %1").arg(e.what()));
    }

}


// 窗口退出邏輯
void Widget::on_Widget_destroyed()
{
    serverthread->stop();
    serverthread->quit();
    serverthread->wait();
}

// 日志
void Widget::new_log(const QString &arg1){
    ui->logoutput->addItem(arg1);
    ui->logoutput->scrollToBottom();
}

// 停止服務
void Widget::on_seriverStopButturn_clicked()
{
    try {
        this->serverthread->stop();
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Error", QString("Failed to stop server: %1").arg(e.what()));
    }
}


void Widget::on_lineEdit_editingFinished()
{
    this->serverthread->sendsubtittle(ui->lineEdit->text());
    ui->lineEdit->setText("");
}


void Widget::on_lineEdit_2_editingFinished()
{
    if(ui->lineEdit_2->text().length() - 2 < 0){
        ui->lineEdit_2->setText("8080");
        this->serverthread->setport(8080);
    }else{
        this->serverthread->setport(ui->lineEdit_2->text().toInt());
    }
}

