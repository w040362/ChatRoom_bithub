#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QTcpSocket_s.cpp"
#include <QDebug>
#include <QDateTime>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    explicit MainWindow(QTcpSocket_s * sock, QWidget *parent = 0, QString name ="");
    ~MainWindow();

private slots:
    void on_send_clicked();
    void recvMessage();
    void on_pushbutton_clicked();

private:
    Ui::MainWindow *ui;
    QTcpSocket_s * sock;
    QString userName;

signals:
    void closeSig();
};

#endif // MAINWINDOW_H
