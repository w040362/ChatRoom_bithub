#ifndef QTCPSOCKET_S
#define QTCPSOCKET_S

#include <string>
#include <windows.h>
#include <QDebug>
#include <QString>
#include <QByteArray>
#include <QTcpSocket>
//#include <QTime>

class QTcpSocket_s: public QTcpSocket {
//private:
//    QTime qtime;
public:
    QByteArray read_s() {
//        qDebug() << QString("QByteArray read_s() is called\n");
        int len = read_s(7).mid(1,4).toInt();
        return read_s(len);
    }
    QByteArray read_s(qint64 Size) {
//        qDebug() << QString("QByteArray read_s(qint64 Size) is called: ") + QString::number(qtime.currentTime().msec());
        char buf[Size];
        unsigned count = 0;

        while(count < Size) {
//            qDebug() << QString("read(char *data, int Size) is called: ") + QString::number(qtime.currentTime().msec());
            count += read(buf + count, Size - count);
            if(bytesAvailable() < Size - count)
                waitForReadyRead();
//            qDebug() << QString("read(char *data, int Size) finish: ") + QString::number(qtime.currentTime().msec());
        }
        return QByteArray(buf, Size);
    }
    void write_s(const char* data) {
//        qDebug() << QString("void write_s(const char* data) is called: ") + QString::number(qtime.currentTime().msec());
        std::string strWit(data);
        std::string strLen("%%");
        int len = strWit.size();
        for(int i = 0; i < 4; i++) {
            if(len == 0) strLen = "0" + strLen;
            else {
                strLen = std::to_string(len%10) + strLen;
                len /= 10;
            }
        }
        strLen = "%" + strLen;

        const char *buf = strLen.c_str();
        for(unsigned count = 0;count < strLen.size();) {
            count += write(buf + count, strLen.size() - count);
        }
        for(int i = 0;i < 10; i++) {
            if(write(strWit.c_str(), strWit.size()) == strWit.size())
                break;
            else {
                Sleep(i * 100);
                qDebug() << QString("Server Busy, retry after ") + QString::number(i * 100) + QString(" ms");
            }
        }

    }
};

#endif







