#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QMainWindow>
#include "QTcpSocket_s.cpp"
#include <QDebug>
#include <QDateTime>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileDialog>
#include <QCheckBox>
#include <QColor>
#include <QLabel>
#include <QScrollBar>
#include <QDateTime>
#include <windows.h>
#include <QSize>
#include <QKeyEvent>
#include <QIcon>
#include <QPixmap>
#include <QFileDialog>
#include <QScrollBar>
#include <QHBoxLayout>
#define BUF_SIZE 1000

namespace Ui {
class chatWindow;
}

class chatWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit chatWindow(QWidget *parent = 0);
    explicit chatWindow(QTcpSocket_s * sock, QString name, QWidget *parent = 0);
    QString getTime();
    void recvMessage();
    void addIconForButton(QPushButton *button, int x, int y, QString path);
    void sendFile();
    ~chatWindow();

protected:
    bool eventFilter(QObject *, QEvent *);

public slots:
    void on_pushButton_clicked();
    void recurRecvMessage();
    void uNameClicked();
    void changeRelation();
    void listFriends(QStringList FriendList);
    void listOnLineUsers(QStringList onlineUserList);

private slots:
    void on_listWidget_clicked(const QModelIndex &index);
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();

private:
    Ui::chatWindow *ui;
    QTcpSocket_s * sock;
    QString userName;
    QString chatPerson;
    int historyFlag;
    QString fileName;
};

#endif // CHATWINDOW_H
