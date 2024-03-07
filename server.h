#ifndef SETVER_H
#define SETVER_H
#include <httplib.h>
#include <QThread>
#include <fstream>
#include <vector>
#include <QDateTime>
#include <QCryptographicHash>

class ServerThread : public QThread {
    Q_OBJECT
public:

    void setport(int port){
        if(!stopsign){
            this->port = port;
            return;
        }
        return;
    }

    void run() override {
        stopsign = 0;
        // 设置读取超时时间（秒）
        svr.set_read_timeout(5, 0);  // 5秒
        // 设置写入超时时间（秒）
        svr.set_write_timeout(5, 0); // 5秒


        // 长轮询的 API
        svr.Get("/api/get", std::bind(&ServerThread::api, this, std::placeholders::_1, std::placeholders::_2));

        // 主頁
        svr.Get("/", std::bind(&ServerThread::mainpage, this, std::placeholders::_1, std::placeholders::_2));

        emit serverStarted(1);

        emit serverLog("starting");
        emit serverLog("localhost server is runing at port " + QString::number(port));

        svr.listen("localhost", port);

        while(!stopsign&&svr.is_running()){
            QObject().thread()->usleep(1000*1000);
        }
        svr.stop();

        emit serverStopped(1);
        emit serverLog("stopped");

        this->exit();
    }

    void stop(){
        stopsign = 1;
        svr.stop();
    }

    void sendsubtittle(const QString message){
        messageList.append(message);
    }
signals:
    void serverStopped(bool);
    void serverStarted(bool);
    void serverLog(QString);

private:
    QList<QString> sessions;
    int port = 8080;
    bool stopsign = 0;
    httplib::Server svr;
    QList<QString> messageList;

    //日志輸出
    void log(const QString &arg1){
        emit serverLog(arg1);
    }

    // 主頁
    void mainpage(const httplib::Request& req /*req*/, httplib::Response& res){
        log("client("+QString::fromStdString(req.remote_addr)+") get mainpage");
        std::ifstream file("./www/indix.html", std::ios::in | std::ios::binary);


        // 生成随机的cookie值
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QString randomString = QString::number(currentDateTime.toMSecsSinceEpoch());

        // 使用哈希函数生成一个加密的cookie值
        QByteArray cookieData = randomString.toUtf8();
        QByteArray hash = QCryptographicHash::hash(cookieData, QCryptographicHash::Md5);
        QString cookieValue = hash.toHex();

        res.set_header("Set-Cookie", "SESSION=" + cookieValue.toStdString());

        if (file.is_open()) {
            std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});
            std::string utf8_content(buffer.begin(), buffer.end());
            res.set_content(utf8_content, "text/html; charset=utf-8");
            file.close();
        } else {
            res.status = 404;
        }
    }

    // 長輪詢api
    void api(const httplib::Request& req, httplib::Response& res){
        if (req.has_header("Cookie")) {
            // 獲取session
            std::string cookie_header = req.get_header_value("Cookie");
            size_t pos = cookie_header.find("SESSION=");
            std::string session_cookie = cookie_header.substr(pos + 8);
            QString session = QString::fromStdString(session_cookie);

            log("client ("+QString::fromStdString(req.remote_addr)+") Get api");

            // 等待消息
            int times = 5;
            while(messageList.isEmpty() && times > 0 && !stopsign){
                QObject().thread()->usleep(1000*1000);
                times--;
            }
            if(messageList.isEmpty()){
                // 返回 502 错误状态码
                res.status = 502;
                res.set_content("Bad Gateway", "text/plain");
                return;
            }

            // 發送消息
            QString message = messageList.takeLast();
            log("send message(to "+QString::fromStdString(req.remote_addr)+"):" + message);
            res.set_content(message.toStdString(), "text/plain; charset=utf-8");

            return;
        }
    }
};
#endif // SETVER_H
