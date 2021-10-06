#include "client.h"
#include "ui_client.h"

Client::Client(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Client)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/image/icon3.jpg"));
    sock = new QTcpSocket_s();
    sock->connectToHost("101.34.125.16", 8899);
    connect(sock, SIGNAL(connected()), this, SLOT(handConnected()));
}

Client::~Client()
{
    delete ui;
    delete sock;
}

/**************************************************/
/*名称：on_launch_clicked
/*描述：点击登录按钮时向服务器发送登录信息
/*参数：
/*返回值：void
/***************************************************/

void Client::on_launch_clicked()
{
    QString str1 = ui->uName->text();
    userName = ui->uName->text();
    QString str2 = ui->pWord->text();
    QString str3 = "#02|"+str1+"|"+str2;
    qDebug() << str3;

    //转换为字符串
    str3.toStdString().c_str();
    sock->write_s(str3.toUtf8());
}

/**************************************************/
/*名称：handConnected
/*描述：连接成功时将登录按钮变为可点击状态，绑定handRead()
/*参数：
/*返回值：void
/***************************************************/

void Client::handConnected()
{
    ui->launch->setEnabled(true);
    connect(sock,SIGNAL(readyRead()),this,SLOT(handRead()));
}

/**************************************************/
/*名称：handRead
/*描述：读取服务器信息，判断登陆是否成功并反馈。
/*参数：
/*返回值：void
/***************************************************/

void Client::handRead()
{
    QByteArray recvArray = sock->read_s();
    QString recvStr(recvArray);
    qDebug()<<recvStr;
    //#02|0|success
    QStringList recvlist = recvStr.split('|');

    if(recvlist[0] == "#02")
    {
        if(QString :: compare(recvlist[1], "000") == 0)// 000:登陆成功
        {
            QMessageBox::information(this, "提示","登陆成功！");
            chatWindow *cw = new chatWindow(sock, userName);
            disconnect(sock, SIGNAL(readyRead()),this,0);
            cw->show();
            this->hide();
        }
        else if(QString :: compare(recvlist[1], "001") == 0)// 001:用户不存在
        {
            QMessageBox::warning(this, "警告","用户不存在");
        }
        else if(QString :: compare(recvlist[1], "002") == 0)// 002:密码错误
        {
            QMessageBox::warning(this, "警告","密码错误");
        }
        else if(QString :: compare(recvlist[1], "003") == 0)// 003:重复登录
        {
            QMessageBox::warning(this, "警告","您已登录");
        }
        else if(QString :: compare(recvlist[1], "004") == 0)// 004:系统错误
        {
            QMessageBox::warning(this, "警告","系统错误，请联系管理员");
        }
    }
}

/**************************************************/
/*名称：on_Register_clicked
/*描述：点击注册按钮时，弹出注册窗口，关闭登录窗口
/*参数：
/*返回值：void
/***************************************************/

void Client::on_Register_clicked()
{
    disconnect(sock, SIGNAL(readyRead()),this,0);
    Register *rg = new Register(sock);
    rg->show();
    connect(rg, SIGNAL(closeSig()), this, SLOT(reLogin()));
    this->hide();
}

/**************************************************/
/*名称：reLogin
/*描述：新注册的用户回到登录界面，重新连接handRead()
/*参数：
/*返回值：void
/***************************************************/

void Client::reLogin()
{
    connect(sock, SIGNAL(readyRead()), this, SLOT(handRead()));
    this->show();
}
