#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>
#include <QDebug>
#include "QTcpSocket_s.cpp"
#include <QFile>
#include <QDesktopWidget>
#include <QMessageBox>
#include "mainwindow.h"
#include "register.h"
#include "chatwindow.h"

namespace Ui {
class Client;
}

class Client : public QDialog
{
    Q_OBJECT

public:
    explicit Client(QWidget *parent = 0);
    ~Client();

private slots:
    void handConnected();
    void handRead();
    void on_launch_clicked();
    void on_Register_clicked();
    void reLogin();

private:
    Ui::Client *ui;
    QTcpSocket_s *sock;
    QString userName;
};

#endif // CLIENT_H
