#include "register.h"
#include "ui_register.h"

Register::Register(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Register)
{
    ui->setupUi(this);
}

Register::Register(QTcpSocket_s * s, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Register)
{
    this->setWindowIcon(QIcon(":/image/icon3.jpg"));

    ui->setupUi(this);
    sock = s;
    connect(sock,SIGNAL(readyRead()),this,SLOT(handRead()));
}

Register::~Register()
{
    delete ui;
    delete sock;
}

void Register::handRead()
{
    // 接收服务器处理结果
    QByteArray recvArray = sock->read_s();
    QString recvStr(recvArray);
    QStringList recvlist = recvStr.split('|');
    qDebug()<<recvStr;

    // 根据结果进行反馈
    if(recvlist[0] == "#01")
    {
        if(QString :: compare(recvlist[1], "000") == 0)    // 000:注册成功，返回登录窗口
        {
            QMessageBox::information(this, "提示","注册成功！");
            disconnect(sock,SIGNAL(readyRead()), 0, 0);
            this->close();
            emit closeSig();    // 发出关闭信号，唤起登录窗口
        }
        else if(QString :: compare(recvlist[1], "001") == 0)   // 001:用户名已存在
        {
            QMessageBox::warning(this, "警告","该用户名已被注册");
        }
        else if(QString :: compare(recvlist[1], "002") == 0)   // 002:系统错误
        {
            QMessageBox::warning(this, "警告","系统错误，请联系管理员");
        }
    }
}

void Register::on_sendRegister_clicked()
{
    QString str1 = ui->uName->text();
    QString str2 = ui->pWord->text();
    QString str3 = ui->pWord_2->text();

    // 检查用户名是否有效
    if(QString::compare(str1, "") == 0)
        QMessageBox::warning(this, "警告", "请输入用户名");

    // 检查两次密码输入是否一致
    else if(QString::compare(str2, str3) != 0)
        QMessageBox::warning(this, "警告", "密码不一致，请检查");

    // 向服务器发送注册信息
    else
    {
        QString registerInformation = "#01|" + str1 + '|' + str2;
        registerInformation.toStdString().c_str();
        sock->write_s(registerInformation.toUtf8());
    }
}
