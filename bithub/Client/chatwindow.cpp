#include "chatwindow.h"
#include "ui_chatwindow.h"

/*************************************************构造函数****************************************************/

chatWindow::chatWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::chatWindow)
{
    ui->setupUi(this);
}

chatWindow::chatWindow(QTcpSocket_s * s, QString name,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::chatWindow)
{
    this->setWindowIcon(QIcon(":/image/icon3.jpg"));

    ui->setupUi(this);
    ui->title->setText("         公共聊天室");                                                       // 默认为公共聊天室
    sock = s;
    userName = name;
    chatPerson = QString("public");
    historyFlag = 0;//没有历史消息
    QString str_uf = "#04|" + userName + "|Get_Online_User";
    str_uf.toStdString().c_str();
    sock->write_s(str_uf.toUtf8());

    connect(sock, SIGNAL(readyRead()),this,SLOT(recurRecvMessage()));

    ui->messageWindow->scrollToBottom();
    ui->lineEdit->installEventFilter(this);                                             // 事件过滤器

//    QString path = ":/image/send1_copy.png";
//    addIconForButton(ui->pushButton, 25, 25, path);

    ui->progressBar->hide();

    QString sss = "\
    QScrollBar:vertical   \
    {                                                               \
        border: none;                                               \
        margin: 0px,0px,0px,0px;                                    \
            border-top-left-radius:10px;\
            border-top-right-radius:0px;\
            border-bottom-left-radius:10px;\
            border-bottom-right-radius:10px;\
        background:   #DADBE5;                                      \
    }                                                               \
                                                                    \
    QScrollBar:vertical:hover                                       \
    {                                                               \
        margin: 0px,0px,0px,0px;                                    \
        background-color: #DADBE5;                             \
    }                                                               \
                                                                    \
    QScrollBar::handle:vertical                                     \
    {                                                               \
        max-width:4px;                                              \
        background:#C6C8D7;                                      \
        height: 40px;                                               \
            border-top-left-radius:10px;\
            border-top-right-radius:10px;\
            border-bottom-left-radius:10px;\
            border-bottom-right-radius:10px;\
    }                                                              \
    QScrollBar::handle:vertical:hover                               \
    {                                                               \
                                                      \
    }                                                               \
    QScrollBar::add-line:vertical                                   \
    {                                                               \
        height:0px;                                                \
        background-color: transparent;                              \
        subcontrol-position:bottom;                                 \
    }                                                               \
    QScrollBar::sub-line:vertical                                   \
    {                                                               \
        height:0px;                                                \
        background-color: transparent;                             \
        subcontrol-position:top;                                    \
    }                                                               \
                                                                    \
    QScrollBar::add-page:vertical                                   \
    {                                                               \
        background-color: rgba(50, 50, 50, 0);                      \
    }                                                               \
                                                                    \
    QScrollBar::sub-page:vertical                                   \
    {                                                               \
        background-color: rgba(50, 50, 50, 0);                      \
    }";
    ui->messageWindow->verticalScrollBar()->setStyleSheet(sss);
    ui->messageWindow->verticalScrollBar()->setFixedWidth(20);


    QString progressBarStyle = "QProgressBar{\
            font:9pt;\
            border-radius:5px;\
            text-align:center;\
            border:1px solid #E8EDF2;\
            background-color: rgb(255, 255, 255);\
            border-color: rgb(180, 180, 180);\
        }\
        QProgressBar:chunk{\
            border-radius:5px;\
            background-color:#1ABC9C;\
        }";
    ui->progressBar->setStyleSheet(progressBarStyle);


    // 默认为public聊天
    QString chatfilePath = "../chatfile";
    QDir chatfile(chatfilePath);
    if(!chatfile.exists())                                                              // 文件夹不存在则创建
        chatfile.mkdir(chatfilePath);

    QFile file("../chatfile/public.txt");
    if(file.exists())
    {
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        ui->messageWindow->clear();
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        if(file.exists())
        {
            file.open(QIODevice::ReadOnly | QIODevice::Text);

            while(1)
            {
                QByteArray t = file.readLine();
                if(t != "----以上为离线消息----\n")
                    t = t + file.readLine();
                qDebug()<<t;
                if(t.isNull())
                    break;
                t.chop(1);
                qDebug()<<t;
                if(t.isNull())
                    break;
                ui->messageWindow->addItem(QString(t));
                ui->messageWindow->scrollToBottom();
            }
            file.close();
        }
    }
}

chatWindow::~chatWindow()
{
    delete ui;
    delete sock;
}

/*************************************************槽函数****************************************************/

void chatWindow::on_pushButton_3_clicked() // 文件
{
    fileName = QFileDialog::getOpenFileName(this,tr("请选择文件"),"../",tr("文本文件(*txt)"));
    if(chatPerson == "public")
        QMessageBox::warning(this, "警告","请勿在群聊中发送文件");// !!!!
    if(fileName.size() != 0 && chatPerson != "public")
        sendFile();
}

void chatWindow::on_pushButton_4_clicked() // 图片
{
    fileName = QFileDialog::getOpenFileName(this,tr("请选择图片"),"../",tr("图片(*png *jpg)"));
     if(chatPerson == "public")
         QMessageBox::warning(this, "警告","请勿在群聊中发送图片");
     if(fileName.size() != 0 && chatPerson != "public")
         sendFile();
}

void chatWindow::recurRecvMessage()
{
    while(sock->bytesAvailable())
        chatWindow::recvMessage();
}


/**************************************************/
/*名称：changeRelation
/*描述：改变好友关系的槽函数,向服务器发送
/*参数：star:用表示好友关系的按钮
/*返回值：void
/***************************************************/

void chatWindow::changeRelation()
{
    int mouse_x = QCursor::pos().x();//鼠标点击处横坐标
    int mouse_y = QCursor::pos().y();//鼠标点击处纵坐标
    QPushButton *nowStar = qobject_cast <QPushButton *>(QApplication::widgetAt(mouse_x, mouse_y));//获取鼠标点击处的控件

    // 父控件是widget
    QWidget *nowLayout =  (QWidget*)(nowStar->parent());
    QList<QPushButton*> cP = nowLayout->findChildren<QPushButton*>();

    QString cPName = cP[1]->text();
    qDebug() << cPName;
    if(cPName == userName)
        return;

    if(nowStar->property("role") == "user")    // 添加好友
    {
        QString str = "#08|000|" + userName + "|" + cPName;
        qDebug() << str;
        str.toStdString().c_str();
        sock->write_s(str.toUtf8());
    }
    else                                                                                // 删除好友
    {
        QString str = "#08|001|" + userName + "|" + cPName;
        qDebug() << str;
        str.toStdString().c_str();
        sock->write_s(str.toUtf8());
    }
    // 刷新好友列表
    QString str = "#04|" + userName + "|Get_Online_User";
    qDebug()<< str;
    str = str.toStdString().c_str();
    sock->write_s(str.toUtf8());
}

/**************************************************/
/*名称：on_pushButtton_clicked
/*描述：点击发送按钮时向服务器发送聊天信息
/*参数：
/*返回值：void
/***************************************************/

void chatWindow::on_pushButton_clicked()
{
    if(chatPerson == "public")                                                          // 发送公共消息
    {
        QString timeStr = getTime();
        QString sendStr = "#05|" + userName + "|" + ui->lineEdit->text() + "|" + timeStr;
        qDebug()<<timeStr;
        sendStr = sendStr.toStdString().c_str();
        sock->write_s(sendStr.toUtf8());
        ui->lineEdit->clear();
    }

    else                                                                                // 发送私人消息
    {
        QString timeStr = getTime();
        qDebug()<<timeStr;

        QString writeStr = userName + " to " + chatPerson + " " + timeStr + "\n" + ui->lineEdit->text();
        qDebug()<<chatPerson;
        QFile file("../chatfile/" + userName + "_" + chatPerson + ".txt");
        file.open(QIODevice::WriteOnly | QIODevice::Append);

        writeStr = writeStr + "\n";
        file.write(writeStr.toUtf8());
        file.close();
        writeStr.chop(1);

        ui->messageWindow->addItem(writeStr);
        ui->messageWindow->scrollToBottom();

        QString sendStr = "#06|" + userName + "|" + chatPerson + "|"  + ui->lineEdit->text() + "|" + timeStr;
        sendStr = sendStr.toStdString().c_str();
        sock->write_s(sendStr.toUtf8());
        ui->lineEdit->clear();
    }
}


/**************************************************/
/*名称：on_uName_clicked
/*描述：用户点击在线列表，确定聊天对象
/*参数：
/*返回值：void
/***************************************************/

void chatWindow::uNameClicked()
{
    int mouse_x = QCursor::pos().x();//鼠标点击处横坐标
    int mouse_y = QCursor::pos().y();//鼠标点击处纵坐标
    QPushButton *nowChatPerson = qobject_cast <QPushButton *>(QApplication::widgetAt(mouse_x, mouse_y));//获取鼠标点击处的控件
    chatPerson = nowChatPerson->text();
    qDebug()<< chatPerson;

    ui->title->setText(chatPerson);                                             // 显示当前聊天对象

    ui->messageWindow->clear();
    QFile file("../chatfile/"+ userName + "_" + chatPerson+".txt");

    QString sendStr = "#07|" + chatPerson + "|" + userName;
    sendStr = sendStr.toStdString().c_str();
    sock->write_s(sendStr.toUtf8());//发送离线消息请求
    if(file.exists())
    {
           file.open(QIODevice::ReadOnly | QIODevice::Text);

           while(1)
           {
               QByteArray t = file.readLine();
               if(t != "----以上为离线消息----\n")
               {
                   t = t + file.readLine();
               }
               qDebug()<<t;
               if(t.isNull())
                   break;
               t.chop(1);
               ui->messageWindow->addItem(QString(t));
               ui->messageWindow->scrollToBottom();
           }
           file.close();
    }

}

void chatWindow::on_pushButton_2_clicked()
{
    QString str_uf = "#04|" + userName + "|Get_Online_User";
    str_uf.toStdString().c_str();
    sock->write_s(str_uf.toUtf8());
}

/*************************************************自定义函数****************************************************/

void chatWindow::sendFile()
{
    QStringList strList = fileName.split("/");
    QString name = strList[strList.count()-1];

    ui->progressBar->show();
    FILE *fp = fopen(fileName.toStdString().c_str(), "rb");
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    int totalBlock = (size%BUF_SIZE == 0 ? size/BUF_SIZE : size/BUF_SIZE + 1);
    fseek(fp, 0, SEEK_SET);

    int cur_block = 0;
    unsigned char c = 0;
    std::string fileEncode;
    std::string fileBit;
    int count = 0;
    while(!feof(fp)) {
        if(fread(&c, 1, 1, fp))
        {
            count ++;
            fileBit = std::to_string(int(c));
            if(fileBit.size() == 3)
                fileEncode = fileEncode + fileBit;
            else if(fileBit.size() == 2)
                fileEncode = fileEncode + "0" + fileBit;
            else
                fileEncode = fileEncode + "00" + fileBit;
        }
        if(count == BUF_SIZE || feof(fp)) {
            std::string strWrite;
            std::string uName = userName.toStdString();
            std::string cPerson = chatPerson.toStdString();
            strWrite = "#09|000|" + uName + "|" + cPerson + "|" + std::to_string(cur_block) + "|" + std::to_string(totalBlock) + "|" + name.toStdString() + "|" + fileEncode;
//            qDebug()<<strWrite;
            sock->write_s(strWrite.c_str());

            fileEncode.clear();
            count = 0;
            cur_block ++;
        }
        ui->progressBar->setValue((cur_block * 100) / totalBlock);
    }
    fclose(fp);
    QMessageBox::information(this, "提示", "传输成功");
    ui->progressBar->hide();
}

/**************************************************/
/*名称：addIconForButton
/*描述：为按钮添加图像
/*参数：button:按钮指针；x, y:图像长、宽；path:图像路径
/*返回值：void
/***************************************************/

void chatWindow::addIconForButton(QPushButton *button, int x, int y, QString path)
{
    QPixmap pixmap;
    pixmap.load(path);
    QPixmap fitpixmap = pixmap.scaled(x, y, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    button->setIcon(QIcon(fitpixmap));
    button->setIconSize(QSize(x, y));
    button->setFlat(true);
    button->setStyleSheet("border: 0px"); //消除边框
    return;
}

/**************************************************/
/*名称：recvMessage
/*描述：从服务器接收用户列表、群聊、私聊、添加好友、删除好友等信息
/*参数：
/*返回值：void
/***************************************************/

void chatWindow::recvMessage()
{
    QByteArray recvArray = sock->read_s();
    qDebug()<<recvArray;
    QString recvStr(recvArray);
    qDebug()<<recvStr;
    QStringList recvList = recvStr.split('|');

    // 接收在线用户列表信息 #04|000|{}
    if(recvList[0] == "#04")                                                            // 在线用户列表
    {
        if(recvList[1] == "000")                                                        // 成功获得在线用户列表
        {
            ui->listWidget->clear();                                                    // 每次获取用户列表前清空当前list
            ui->listWidget->addItem(new QListWidgetItem("         公共聊天室"));
            if(recvList[2] == "{}")                                                 // 没有好友
            {
                recvList[3].remove(QRegularExpression("[{}]"));
                QStringList onlineUserList = recvList[3].split(',');
                listOnLineUsers(onlineUserList);
            }
            else
            {
                recvList[2].remove(QRegularExpression("[{}]"));                     // 使用正则表达式清除首尾{}
                recvList[3].remove(QRegularExpression("[{}]"));
                QStringList friendList = recvList[2].split(',');
                QStringList onlineUserList = recvList[3].split(',');
                // 显示朋友
                listFriends(friendList);
                // 显示在线用户
                listOnLineUsers(onlineUserList);
            }
        }
        else if(recvList[2] == "001")                                                   // 获取在线用户列表失败
        {
            qDebug()<<QString("dbFailure");
            QMessageBox::warning(this, "警告","系统错误，请联系管理员");
        }
    }

    // 公共聊天
    else if(recvList[0] == "#05")
    {
        if(recvList[1] == "000")//成功
       {
           qDebug()<<QString("success");

       }else if(recvList[1] == "001")//失败
       {
           qDebug()<<QString("fail");
       }else//返回信息
       {
           QString writeStr = recvList[1] + "  " + recvList[3] + "\n" + recvList[2];
           if(chatPerson == "public")//正在公共聊天中
           {
               ui->messageWindow->addItem(writeStr);
               ui->messageWindow->scrollToBottom();
           }
           else
               QMessageBox::information(this, "提示",recvList[1] + "在公共聊天区域发送了一条消息");
           QFile file("../chatfile/public.txt");
           file.open(QIODevice::WriteOnly | QIODevice::Append);
           writeStr = writeStr + "\n";
           file.write(writeStr.toUtf8());
           file.close();
       }
    }

    // 私人聊天
    else if(recvList[0] == "#06")
    {
        if(recvList[1] == "000")//成功
          {
              qDebug()<<QString("success");
          }else if(recvList[1] == "001")//不在线
          {
              qDebug()<<QString("offline");
          }else if(recvList[1] =="002")//失败
          {
              qDebug()<<QString("fail");
          }else//返回信息
          {
                if(recvList[1] != recvList[2])
                {
                  QString writeStr = recvList[1] + " to " + recvList[2] + "  " + recvList[4] + "\n" + recvList[3];
                  if(chatPerson == recvList[1])
                  {
                      ui->messageWindow->addItem(writeStr);
                      ui->messageWindow->scrollToBottom();
                  }
                  else
                      QMessageBox::information(this, "提示",recvList[1] + "发送了一条消息");
                  QFile file("../chatfile/" + recvList[2] + "_" + recvList[1] + ".txt");//这里有修改
                  file.open(QIODevice::WriteOnly | QIODevice::Append);
                  writeStr = writeStr + "\n";
                  file.write(writeStr.toUtf8());
                  file.close();
                }
          }
    }

    // 离线信息收发
    else if(recvList[0] == "#07")
    {
        if(recvList[1] == "Offline_Message_Finish")
        {
            if(historyFlag == 1)
            {
                ui->messageWindow->clear();
                QFile file("../chatfile/" + userName + "_" + chatPerson + ".txt");

                file.open(QIODevice::ReadOnly | QIODevice::Text);
                if(file.exists())
                {
                    file.open(QIODevice::ReadOnly | QIODevice::Text);

                    while(1)
                    {
                        QByteArray t = file.readLine();
                        if(t != "----以上为离线消息----\n")
                            t = t + file.readLine();
                        qDebug()<<t;
                        if(t.isNull())
                            break;
                        t.chop(1);
                        ui->messageWindow->addItem(QString(t));
                        ui->messageWindow->scrollToBottom();
                    }
                    file.close();
                }
                ui->messageWindow->addItem(QString("----以上为离线消息----\n"));
                ui->messageWindow->scrollToBottom();
            }
            historyFlag = 0;
        }
        else
        {
            historyFlag = 1;
            QString writeStr = recvList[1] + " to " + recvList[2] + "  " + recvList[4] + "\n" + recvList[3];
            QFile file("../chatfile/" + recvList[2] + "_" + recvList[1] + ".txt");
            file.open(QIODevice::WriteOnly | QIODevice::Append);
            writeStr = writeStr + "\n";
            file.write(writeStr.toUtf8());
            file.close();
        }
    }

    // 添加、删除好友
    else if(recvList[0] == "#08")
    {
        // 添加好友
        if(recvList[1] == "000")
            QMessageBox::information(this, "提示","成功添加好友");
        // 删除好友
        else if(recvList[1] == "010")
            QMessageBox::information(this, "提示","成功删除好友");
    }

    // 文件传输
    else if(recvList[0] == "#09")
    {
        if(recvList[1] == "001"){
            // recvList[2] from_u
            // recvList[3] to_u
            // recvList[4] cur_block
            // recvList[5] tot_block
            // recvList[6] file_name
            // recvList[7] file_block

            QString path;
            path.append("..\\recvfile\\");
            path.append(recvList[3]);
            path.append("\\");
            path.append(recvList[2]);
            qDebug()<<path;

            QString cur_block = recvList[5];
            QString tot_block = recvList[6];
            int cur = cur_block.toInt();
            int tot = tot_block.toInt();

            QDir chatfile;
            if(!chatfile.exists(path))
                {// 文件夹不存在则创建
                chatfile.mkpath(path);
                qDebug()<<"123";
            }
            path.append("\\");
            path.append(recvList[4]);
            qDebug()<<path;
            FILE* out = fopen(path.toStdString().c_str(), "ab");
//            FILE* out = fopen("D:/b.txt", "ab");
            QString Qbuf = recvList[recvList.count()-1];
            Qbuf.toStdString().c_str();
            QByteArray nQbuf = Qbuf.toUtf8();
            char *buf = nQbuf.data();
            char bitWrite[4] = {0};

            for (int i = 0; i < nQbuf.size()/3; i++) {
                bitWrite[0] = buf[i * 3];
                bitWrite[1] = buf[i * 3 + 1];
                bitWrite[2] = buf[i * 3 + 2];
                fprintf(out, "%c", atoi(bitWrite));
            }
            fclose(out);
            if(cur == tot-1){
                qDebug() << "Successfully Received";
                QMessageBox::information(this,"提示",recvList[2] + "发送了" +recvList[4]);
            }
        }
    }
}


/**************************************************/
/*名称：eventFilter
/*描述：用户敲击回车时发送消息
/*参数：
/*返回值：void
/***************************************************/

bool chatWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->lineEdit && event->type() == QEvent::KeyPress)
    {
        int key = (static_cast<QKeyEvent *>(event))->key();
        if (Qt::Key_Return == key || Qt::Key_Enter == key)
        {
            chatWindow::on_pushButton_clicked();
            qDebug() << "Enter pressed";
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

/**************************************************/
/*名称：getTime
/*描述：获取当前时间
/*参数：
/*返回值：QString
/***************************************************/

QString chatWindow::getTime()
{
    QDateTime currentDatetime =QDateTime::currentDateTime();
    QString currentDate =currentDatetime.toString("yyyy-MM-dd hh:mm:ss");
    qDebug()<<currentDate;
    return currentDate;
}

/**************************************************/
/*名称：listFriends
/*描述：展示好友列表
/*参数：friendList:好友列表
/*返回值：void
/***************************************************/

void chatWindow::listFriends(QStringList friendList)
{
    for(int i = 0; i<friendList.count(); i++)
    {
        // 创建当前行组件
        QListWidgetItem *item=new QListWidgetItem;
        item->setSizeHint(QSize(10,50));
        ui->listWidget->addItem(item);

        // 声明组件信息
        QWidget *w = new QWidget;
        QHBoxLayout *layout=new QHBoxLayout;
        QPushButton* star = new QPushButton;
        QString nowChatPerson = friendList[i];
        QPushButton *uName = new QPushButton;

        // 填充组件
        QString path = ":/image/star(1)_copy.png";
        addIconForButton(star, 20, 20, path);
        uName->setText(nowChatPerson);
        star->setProperty("role", "friend");

        // 排版
        star->setFixedSize(20, 30);
        uName->setFixedHeight(30);
        star->setStyleSheet("border-bottom-left-radius:0px;border-top-right-radius:0px;border-top-left-radius:0px;border-bottom-right-radius:0px");
        uName->setStyleSheet("border-bottom-left-radius:0px;border-top-right-radius:0px;border-top-left-radius:0px;border-bottom-right-radius:0px");
        uName->setStyleSheet("font-family: Microsoft Yahei; font: bold; font-size:14; color:rgb(255, 255, 255)");
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);

        // 为好友按钮绑定槽函数
        connect(star, SIGNAL(clicked()), this, SLOT(changeRelation()));
        connect(uName, SIGNAL(clicked()), this, SLOT(uNameClicked()));

        // 展示
        layout->addWidget(star);
        layout->addWidget(uName);
        w->setLayout(layout);
        ui->listWidget->setItemWidget(item,w);
    }
}

/**************************************************/
/*名称：listOnLineUsers
/*描述：展示非好友在线用户列表
/*参数：onLineUserList:非好友在线用户列表
/*返回值：void
/***************************************************/

void chatWindow::listOnLineUsers(QStringList onlineUserList)
{
    for(int i = 0; i<onlineUserList.count(); i++)
    {
        // 创建当前行组件
        QListWidgetItem *item=new QListWidgetItem;
        item->setSizeHint(QSize(50,50));
        ui->listWidget->addItem(item);

        // 声明组件信息
        QWidget *w = new QWidget;
        QHBoxLayout *layout = new QHBoxLayout;
        QPushButton* star = new QPushButton;  
        QString nowChatPerson = onlineUserList[i];
        QPushButton *uName = new QPushButton;

        // 填充组件
        QString path = ":/image/star(2)_copy.png";
        addIconForButton(star, 20, 20, path);
        uName->setText(nowChatPerson);
        star->setProperty("role", "user");

        // 排版
        star->setFixedSize(20, 30);
        uName->setFixedHeight(30);
        star->setStyleSheet("border-bottom-left-radius:0px;border-top-right-radius:0px;border-top-left-radius:0px;border-bottom-right-radius:0px");
        uName->setStyleSheet("border-bottom-left-radius:0px;border-top-right-radius:0px;border-top-left-radius:0px;border-bottom-right-radius:0px");
        uName->setStyleSheet("font-family: Microsoft Yahei; font: bold; font-size:14; color:rgb(255, 255, 255)");
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);

        // 为好友按钮绑定槽函数
        connect(star, SIGNAL(clicked()), this, SLOT(changeRelation()));
        connect(uName, SIGNAL(clicked()), this, SLOT(uNameClicked()));

        // 展示
        layout->addWidget(star);
        layout->addWidget(uName);
        w->setLayout(layout);
        ui->listWidget->setItemWidget(item,w);
    }
}

void chatWindow::on_listWidget_clicked(const QModelIndex &index)
{
    qDebug()<<index.data().toString();
    chatPerson = index.data().toString();

    ui->messageWindow->clear();
    QFile file("../chatfile/public.txt");
    if(file.exists())
    {
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        while(1)
        {
            QByteArray t = file.readLine();
            if(t != "----以上为离线消息----\n")
                t = t + file.readLine();   
            qDebug()<<t;
            if(t.isNull())
                break;
            t.chop(1);
            ui->messageWindow->addItem(QString(t));
            ui->messageWindow->scrollToBottom();
        }
        file.close();
    }

    if(index.data().toString() == "         公共聊天室")
    {
        ui->title->setText("公共聊天室");
        chatPerson = "public";
    }
}


