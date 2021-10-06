#ifndef REGISTER_H
#define REGISTER_H

#include <QMainWindow>
#include "QTcpSocket_s.cpp"
#include <QMessageBox>
#include <QDebug>

namespace Ui {
class Register;
}

class Register : public QMainWindow
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = 0);
    explicit Register(QTcpSocket_s * sock, QWidget *parent = 0);
    ~Register();

private slots:
    void handRead();
    void on_sendRegister_clicked();

private:
    Ui::Register *ui;
    QTcpSocket_s * sock;

signals:
    closeSig();
};

#endif // REGISTER_H
