#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

/**************************************************/
/*名称：insertEmo
/*描述：将图片文件名转为HTML形式，可以直接插入图片
/*参数：图片文件名fName
/*返回值：Qstring
/**************************************************/
QString insertEmo(QString fName)
{
    QString imgPath = ":/" + fName;
    imgPath = QString("<img src=\"%1\"/>").arg(imgPath);
    return imgPath;
}

MainWindow::MainWindow(QTcpSocket_s * s, QWidget *parent, QString name) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    userName = name;
    qDebug()<<userName;
    ui->textBrowser_2->insertHtml(insertEmo("logo.jpg"));

    sock = s;
    connect(sock, SIGNAL(readyRead()),this,SLOT(recvMessage()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

/**************************************************/
/*名称：on_send_clicked()
/*描述：发送按钮的槽函数，将发送信息转为协议要求的字符串
/*参数：
/*返回值：void
/***************************************************/

void MainWindow::on_send_clicked()
{
    QDateTime currentDatetime =QDateTime::currentDateTime();
    QString currentDate =currentDatetime.toString("yyyyMMddhhmmss");
    qDebug()<<currentDate;
    QString sendData = ui->lineEdit->text();
    sendData = "#05|" + userName + "|" + sendData + "|" + currentDate;
    sendData.toStdString().c_str();
    sock->write_s(sendData.toUtf8());
}

void MainWindow::on_pushbutton_clicked()
{
    this->close();
    emit closeSig();
}

/**************************************************/
/*名称：recvMessage()
/*描述：在聊天界面接收信息并判断信息类型
/*参数：
/*返回值：void
/***************************************************/

void MainWindow::recvMessage()
{
    QByteArray recvArray = sock->readAll();
    QString recvStr(recvArray);
    // #05|User0Name|Message|Time
    QStringList recvlist = recvStr.split('|');

    if(recvlist[0] == "#04")//获取在线用户列表 #04|000|{user1,user2,user3,user4}
    {
        //qDebug()<<recvlist[2];
        if(recvlist[2] == "{}")//用户列表为空 #04|000|{}
        {
            qDebug()<<QString("no users");
        }else
        {
            ui->listWidget->clear(); // 每次获取用户列表清空当前list
            QStringList onlineUserList = recvlist[2].split(',');
            for(int i = 0; i<onlineUserList.count(); i++)
            {
                //qDebug()<<onlineUserList[i];
                ui->listWidget->addItem(new QListWidgetItem(onlineUserList[i]));
            }
        }

    }else//获取聊天消息
    {
        if(recvlist[1] == "000")//消息发送成功 #05|000|Success
        {
            qDebug()<<QString("success");
        }else if (recvlist[1] == "001")//消息发送失败 05|001|Fail
        {
            qDebug()<<QString("Fail");
        }else//接收用户信息 #05|User0Name|Message|Time
        {
            QString speakerInformation = recvlist[3] + ' ' + recvlist[1];
            ui->textBrowser->append(speakerInformation);
            QString message = recvlist[2] + '\n';
            ui->textBrowser->append(message);
        }
    }
}

