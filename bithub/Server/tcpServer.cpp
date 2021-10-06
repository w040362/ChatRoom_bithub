#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <queue>

#define BUF_SIZE 3000
// #define BUF_SIZE 4

struct User {
    std::string name;
    int confd;
};

struct File {
    std::string uName0;
    std::string uName1;
    std::string fileName;
    long total_block;
    File(const std::string &uName0, const std::string &uName1, const std::string &fileName, const long &total_block): 
        uName0(uName0), uName1(uName1), fileName(fileName) ,total_block(total_block) {};
    File(const struct File &file) :
        uName0(file.uName0), uName1(file.uName1), fileName(file.fileName) ,total_block(file.total_block) {};
};

class Users {
private:
    std::vector<struct User> users;
public:
    bool isUser(const std::string &name) { // 判断用户是否已经在线
        for(int i = 0; i < users.size(); i++) {
            if(users[i].name == name) 
                return true;
        }
        return false;
    }
    
    void addUser(const std::string &name, const int &confd) { // 添加用户
        struct User user;
        user.name = name;
        user.confd = confd;
        users.push_back(user);
    }
    
    void delUser(const int &confd) { // 删除用户 用户退出登录
        for(int i = 0; i < users.size(); i++) {
            if(users[i].confd == confd) {
                users.erase(users.begin() + i);
                return;
            }
        }
    } 
    
    int getConfd(const std::string &name) { // name搜索用户confd, 返回值：int -1异常,其他值为confd
        for(int i = 0; i < users.size(); i++) {
            if(users[i].name == name) 
                return users[i].confd;
        }
        return -1;
    } 
    
    std::string getName(const int &confd) { // confd搜索用户name
        for(int i = 0; i < users.size(); i++) {
            if(users[i].confd == confd)
                return users[i].name;
        return NULL;
        }
    }

    std::vector<int> getAllConfd() { // 获取所有用户confd
        std::vector<int> vecConfd;
        for(int i = 0; i < users.size(); i++) 
            vecConfd.push_back(users[i].confd);
        return vecConfd;
    }
    
    std::vector<std::string> getAllName() { // 获取所有用户name
        std::vector<std::string> vecName;
        for(int i = 0; i < users.size(); i++) 
            vecName.push_back(users[i].name);
        return vecName;
    }
};


Users users;
std::queue<struct File> files;
std::vector<std::string> readBuf;

/**************************************************/
/*名称：strSplit
/*描述：字符串切割
/*参数：str完整字符串,pattern模式串
/*返回值：vector<string>
/***************************************************/
std::vector<std::string> strSplit(const std::string &str, const std::string &pattern) {
    std::vector<std::string> ret;
    if (pattern.empty())
        return ret;
    size_t start = 0, index = str.find_first_of(pattern, 0);
    while (index != str.npos)
    {
        if (start != index)
            ret.push_back(str.substr(start, index - start));
        start = index + 1;
        index = str.find_first_of(pattern, start);
    }
    if (!str.substr(start).empty())
        ret.push_back(str.substr(start));
    return ret;
}


/**************************************************/
/*名称：write_s
/*描述：字符串计算长度并发送
/*参数：confd, s待发送的字符串, len字符串长度
/*返回值：void
/***************************************************/
void write_s(int confd, const char *s, int len) {
    std::string buf("%%");
    if(len <= 0) 
        return;
    for(int i = 0; i < 4; i++) {
        if(len == 0) buf = "0" + buf;
        else {
            buf = std::to_string(len%10) + buf;
            len /= 10;
        }
    }
    buf = "%" + buf + std::string(s);
    // printf("write_s: %s\n", buf.c_str());
    write(confd, buf.c_str(), buf.size());
    return;
}


/*********************************功能********************************************/

/**************************************************/
/*名称：userReg
/*描述：用户注册
/*参数：buf格式 #01|uName|pWord, mysql数据库连接符
/*返回值：int
/***************************************************/
int userReg(const std::string &buf, MYSQL &mysql) {
    std::vector<std::string> vecStr = strSplit(buf,"|");
    std::string uName = vecStr[1];
    std::string pWord = vecStr[2];

    MYSQL_RES *result;
    MYSQL_ROW row;
    std::string sqlStr;
    sqlStr = "CALL userReg('"+uName+"','"+pWord+"',@state);";
    if(mysql_query(&mysql, sqlStr.c_str()) != 0) {
        printf("query error\n");
        return 2; // ??
    }
    sqlStr = "select @state;";
    if(mysql_query(&mysql, sqlStr.c_str()) != 0) {
        printf("query error\n");
        return 2; // ??
    }
    
    result = mysql_store_result(&mysql);
    std::string stateStr = mysql_fetch_row(result)[0];
    mysql_free_result(result);
    // "1"->1, "0"->0
    return atoi(stateStr.c_str());
}


/**************************************************/
/*名称：userLog
/*描述：用户登录
/*参数：buf格式 #02|uName|pWord, confd连接描述符, mysql数据库连接符
/*返回值：int
/***************************************************/
int userLog(const std::string &buf,const int &confd, MYSQL &mysql) {
    std::vector<std::string> vecStr = strSplit(buf,"|");
    std::string uName = vecStr[1];
    std::string pWord = vecStr[2];

    MYSQL_RES *result;
    MYSQL_ROW row;
    std::string sqlStr;
    sqlStr = "CALL userLog('"+uName+"','"+pWord+"',@state);";
    if(mysql_query(&mysql, sqlStr.c_str()) != 0) {
        printf("query error\n");
        return 4; // ??
    }
    sqlStr = "select @state;";
    if(mysql_query(&mysql, sqlStr.c_str()) != 0) {
        printf("query error\n");
        return 4; // ??
    }
    result = mysql_store_result(&mysql);
    std::string stateStr = mysql_fetch_row(result)[0];
    mysql_free_result(result);
    // "1"->1, "0"->0

    if(atoi(stateStr.c_str()) == 0) { // 只要密码正确就返回0,处理3 
        if(users.isUser(uName))
            return 3;
        users.addUser(uName, confd);
        return 0;
    } 
    return atoi(stateStr.c_str());
}


/**************************************************/
/*名称：alterPwd
/*描述：用户修改密码
/*参数：buf格式 #03|uName|npWord, mysql数据库连接符
/*返回值：int
/***************************************************/
int alterPwd(const std::string &buf, MYSQL &mysql) {
    std::vector<std::string> vecStr = strSplit(buf,"|");
    std::string uName = vecStr[1];
    std::string npWord = vecStr[2];
    
    MYSQL_RES *result;
    MYSQL_ROW row;
    std::string sqlStr;
    sqlStr = "CALL alterPwd('"+uName+"','"+npWord+"',@state);";
    if(mysql_query(&mysql, sqlStr.c_str()) != 0) {
        printf("query error\n");
        return 2; // ??
    }
    sqlStr = "select @state;";
    if(mysql_query(&mysql, sqlStr.c_str()) != 0) {
        printf("query error\n");
        return 2; // ??
    }
    result = mysql_store_result(&mysql);
    std::string stateStr = mysql_fetch_row(result)[0];
    mysql_free_result(result);
    // "1"->1, "0"->0
    return atoi(stateStr.c_str());
}


/**************************************************/
/*名称：getFriendList
/*描述：用户uNmae0查询好友列表
/*参数：uName, mysql数据库连接符
/*返回值：std::vector<std::string>
/***************************************************/
std::vector<std::string> getFriendList(const std::string &uName, MYSQL &mysql) {
    std::vector<std::string> vecRtn;
    std::string strQue = "SELECT user2 FROM bithub.friends_list WHERE user1 = '" + uName + "';";
    MYSQL_RES *result;
    MYSQL_ROW row;

    if(mysql_query(&mysql, strQue.c_str()) != 0) {
            printf("query error: getFriendList\n");
            return vecRtn; 
    }
    result = mysql_store_result(&mysql);
    while((row = mysql_fetch_row(result))) {
        vecRtn.push_back(std::string(row[0]));
    }
    mysql_free_result(result);
    return vecRtn;
}


/**************************************************/
/*名称：getList
/*描述：获取在线用户列表
/*参数：buf = #04|uName|Get_Online_User, strList返回的用户名列表格式 {Friend1,Friend2,...}|{User1,User2,User3...}
/*返回值：int
/***************************************************/
int getList(const std::string &buf, std::string &strList, MYSQL &mysql) {
    std::string uName = strSplit(buf, "|")[1];
    std::vector<std::string> vecAll = users.getAllName();
    std::vector<std::string> vecFriend = getFriendList(uName, mysql);
    
    strList = "{";
    for(int i = 0; i < vecFriend.size(); i++) {
        strList = strList + vecFriend[i] + ",";
    }
    if(strList[strList.size() - 1] == ',') 
        strList[strList.size() - 1] = '}';
    else
        strList = strList + "}";

    strList = strList + "|{";
    for(int i = 0; i < vecAll.size(); i++) {
        int flag = 1;
        for(int j = 0; j < vecFriend.size(); j++) {
            if(vecAll[i] == vecFriend[j]) {
                flag = 0;
                break;
            }
        }
        if(flag)
            strList = strList + vecAll[i] + ",";
    }
    if(strList[strList.size() - 1] == ',') 
        strList[strList.size() - 1] = '}';
    else
        strList = strList + "}";
    return 0;
}


/**************************************************/
/*名称：sendGlobal
/*描述：用户向所有在线用户发送消息, mysql数据库连接符
/*参数：buf = #05|User0Name|Message|LocalTime, mysql数据库连接符
/*返回值：int
/***************************************************/
int sendGlobal(const std::string &buf) {
    // 包括 S-> C1
    std::vector<int> confds = users.getAllConfd();
    for(int i = 0;i < confds.size();i++) {
        write_s(confds[i], buf.c_str(), buf.size());
    }
    return 0;
}


/**************************************************/
/*名称：storeOfflineMessage
/*描述：用户向服务器存储离线消息
/*参数：接受者用户名uName1, buf格式#06|uName0|uName1|Message|Time, mysql数据库连接符
/*返回值：int 0正常, -1数据库错误
/***************************************************/
int storeOfflineMessage(const std::string &uName0 ,const std::string &uName1,const std::string &buf, MYSQL &mysql) {
    std::string sqlStr;
    sqlStr = "INSERT INTO message_board VALUES('" + uName0 + "', '" + uName1 + "', '" + buf + "');";
    if(mysql_query(&mysql, sqlStr.c_str()) != 0) {
        printf("query error\n");
        return -1;
    }
    return 0;
}

/**************************************************/
/*名称：readOfflineMessage
/*描述：用户向服务器读取离线消息
/*参数：buf格式#07|uName0|uName1, confd, mysql数据库连接符
/*返回值：int 0正常, -1数据库错误
/***************************************************/
int readOfflineMessage(const std::string &buf,const int &confd,MYSQL &mysql) {
    std::vector<std::string> vecStr = strSplit(buf, "|");
    std::string strQue = "SELECT buf FROM message_board WHERE from_user = '" + vecStr[1] + "'and to_user = '" + vecStr[2] + "';";
    MYSQL_RES *result;
    MYSQL_ROW row;

    if(mysql_query(&mysql, strQue.c_str()) != 0) {
        printf("query error\n");
        return -1;
    }
    result = mysql_store_result(&mysql);
    while((row = mysql_fetch_row(result))) {
        row[0][2] = '7'; // 数据库中功能代码为06
        write_s(confd, row[0], std::string(row[0]).size());
    }
    mysql_free_result(result);
    
    strQue = "DELETE FROM message_board WHERE from_user = '" + vecStr[1] + "'and to_user = '" + vecStr[2] + "';";
    if(mysql_query(&mysql, strQue.c_str()) != 0) {
        printf("query error\n");
        return -1;
    }
    return 0;
}


/**************************************************/
/*名称：sendOne
/*描述：用户向另一用户发送消息,格式#06|uName0|uName1|Message
/*参数：buf格式#06|uName0|uName1|Message, mysql数据库连接符
/*返回值：int
/***************************************************/
int sendOne(const std::string &buf,MYSQL &mysql) {
    std::vector<std::string> vecStr = strSplit(buf, "|");

    int confd = users.getConfd(vecStr[2]);
    if(confd == -1) {
        if(storeOfflineMessage(vecStr[1], vecStr[2], buf, mysql) == -1) 
            return 2;
        return 1;
    }
    write_s(confd, buf.c_str(), buf.size());
    return 0;
}


/**************************************************/
/*名称：addFriend
/*描述：用户uNmae0和uName1相互添加为好友
/*参数：buf格式#08|000|uName0|uName1, mysql数据库连接符
/*返回值：int
/***************************************************/
int addFriend(const std::string &buf, MYSQL &mysql) {
    std::vector<std::string> vecStr = strSplit(buf, "|");
    std::string uName0 = vecStr[2];
    std::string uName1 = vecStr[3];

    MYSQL_RES *result;
    MYSQL_ROW row;
    std::string sqlStr = "CALL addFriend('"+uName0+"','"+uName1+"',@state);";
    if(mysql_query(&mysql, sqlStr.c_str()) != 0) {
        printf("query error\n");
        return 3;
    }
    sqlStr = "select @state;";
    if(mysql_query(&mysql, sqlStr.c_str()) != 0) {
        printf("query error\n");
        return 3;
    }
    result = mysql_store_result(&mysql);
    std::string stateStr = mysql_fetch_row(result)[0];
    mysql_free_result(result);
    return atoi(stateStr.c_str());
}


/**************************************************/
/*名称：delFriend
/*描述：用户uNmae0和uName1双删
/*参数：buf格式#08|001|uName|uName1, mysql数据库连接符
/*返回值：int
/***************************************************/
int delFriend(const std::string &buf, MYSQL &mysql) {
    std::vector<std::string> vecStr = strSplit(buf, "|");
    std::string uName0 = vecStr[2];
    std::string uName1 = vecStr[3];

    MYSQL_RES *result;
    MYSQL_ROW row;
    std::string sqlStr = "CALL delFriend('"+uName0+"','"+uName1+"',@state);";
    if(mysql_query(&mysql, sqlStr.c_str()) != 0) {
        printf("query error\n");
        return 2;
    }
    sqlStr = "select @state;";
    if(mysql_query(&mysql, sqlStr.c_str()) != 0) {
        printf("query error\n");
        return 2;
    }
    result = mysql_store_result(&mysql);
    std::string stateStr = mysql_fetch_row(result)[0];
    mysql_free_result(result);
    return atoi(stateStr.c_str());
}


/**************************************************/
/*名称：ReceiveFile
/*描述：用户向服务器发送文件,存到数据库
/*参数：buf格式#09|000|uName0|uName1|current_block|total_block|filename|fileblock   
/*返回值：int
/***************************************************/
int receiveFile(const std::string &buf) {
    std::vector<std::string> vecStr = strSplit(buf, "|");
    // printf("%s\n",("./files/" + vecStr[3] + "/" + vecStr[2] + "/" + vecStr[6]).c_str());
    
    // 创建文件夹和文件
    system(("mkdir -p ./files/" + vecStr[3] + "/" + vecStr[2]).c_str());
    if(atol(vecStr[4].c_str()) == 0) 
        system(("> ./files/" + vecStr[3] + "/" + vecStr[2]  + "/" + vecStr[6]).c_str());
    
    int fd = open(("./files/" + vecStr[3] + "/" + vecStr[2] + "/" + vecStr[6]).c_str(), O_WRONLY|O_APPEND);
    if(fd == -1) 
        return 1;
    write(fd, vecStr[7].c_str(), vecStr[7].size());
    close(fd);

    
    if(atol(vecStr[4].c_str()) % 50 == 0)
        printf("ReceiveFile: %ld, %ld\n", atol(vecStr[4].c_str()), atol(vecStr[5].c_str()));
    // 服务端文件接收完毕
    if(atol(vecStr[4].c_str()) == atol(vecStr[5].c_str()) - 1) {
        struct File file(vecStr[2], vecStr[3], vecStr[6], atol(vecStr[5].c_str()));
        files.push(file);
        // printf("Finish\n");
    }
    return 0;
}


void sendFile() {
    // files队列为空,所有文件传输完毕
    if(files.empty())
        return;
    struct File &file = files.front();

    // 接收用户离线,将文件任务放到队列尾
    if(!users.isUser(file.uName1)) {
        // files.push(file);
        files.pop();
        return;
    }

    // 用户在线,开始发文件
    int confd = users.getConfd(file.uName1);
    int fd = open(("./files/" + file.uName1 + "/" + file.uName0 + "/" + file.fileName).c_str(), O_RDONLY);
    char buf[BUF_SIZE + 1] = {0};
    std::string strRtn;
    for(long i = 0; i < file.total_block; i++) {
        read(fd, buf, BUF_SIZE);
        strRtn = "#09|001|" + file.uName0 + "|" + file.uName1 + "|" + file.fileName + "|" + std::to_string(i) + "|" + std::to_string(file.total_block) + "|" + buf;
        write_s(confd, strRtn.c_str(), strRtn.size());
        // usleep(100 * 1000);
        if(i % 50 == 0)
            printf("sendFile: %ld, %ld\n", i, file.total_block);
    }
    close(fd);
    files.pop();
    return;
}


/**************************************************/
/*名称：handMessage
/*描述：处理客户端缓存区字符串
/*参数：buf = #??|?|...
/*返回值：int 返回0正常, -1异常
/***************************************************/
int handMessage(const std::string &buf,const int &confd) {
    //------------------暂时处理客户端关闭时发送空消息---------------------
    if(buf.size() < 3) {
        printf("接收到空串\n");
        return -1;
    } 
    //------------------------------------------------------------------
    MYSQL mysql;
    mysql_init(&mysql);
    if(mysql_real_connect(&mysql, "127.0.0.1", "root", "root", "bithub", 0, NULL, 0) == NULL) {
        printf("connect error: %s\n", mysql_error(&mysql));
        write_s(confd, "#00|002|dbFailure", 17);
        return -1;
    }
    
    int state;
    std::string strRtn;
    switch (atoi(buf.substr(1,2).c_str())) {
    
    case 1: { // 注册
        switch (userReg(buf, mysql)) {
            case 0: {
                strRtn.assign("#01|000|Success");
                break;
            }
            case 1: {
                strRtn.assign("#01|001|User_Existed");
                break;
            }
            default: { // 应该只有2, 大概没有其他的值了
                strRtn.assign("#01|002|dbFailure");
                break;
            }
        }
        write_s(confd, strRtn.c_str(), strRtn.size());
        break;
    }
    case 2: { // 登录
        switch (userLog(buf, confd, mysql)) {
            case 0: {
                strRtn.assign("#02|000|Success");
                break;
            }
            case 1: {
                strRtn.assign("#02|001|User_Not_Exist");
                break;
            }
            case 2: {
                strRtn.assign("#02|002|passWord_Error");
                break;
            }
            case 3: {
                strRtn.assign("#02|003|already_Login");
                break;
            }
            default: { // 应该只有4, 大概没有其他的值了
                strRtn.assign("#02|004|dbFailure");
                break;
            }
        }
        write_s(confd, strRtn.c_str(), strRtn.size());
        break;
    }
    case 3: { // 改密码
        switch (alterPwd(buf, mysql)) {
            case 0: {
                strRtn.assign("#03|000|Success");
                break;
            }
            case 1: {
                strRtn.assign("#03|001|Same_passWord");
                break;
            }
            default: { // 应该只有2, 大概没有其他的值了
                strRtn.assign("#03|002|dbFailure");
                break;
            }
        }
        write_s(confd, strRtn.c_str(), strRtn.size());
        break;
    }
    case 4: { // 获取在线用户列表
        state = getList(buf, strRtn, mysql);
        if(state == 0)
            strRtn = "#04|000|" + strRtn;
        write_s(confd, strRtn.c_str(), strRtn.size());
        break;
    }
    case 5: { // 大型聊天室
        state = sendGlobal(buf);
        if(state == 0)
            strRtn.assign("#05|000|Success");
        else // 应该只有1, 大概没有其他的值了
            strRtn.assign("#05|001|Fail");
        write_s(confd, strRtn.c_str(), strRtn.size());
        break;
    }
    case 6: { // 一对一聊天
        switch (sendOne(buf, mysql)) {
            case 0: {
                strRtn.assign("#06|000|Success");
                break;
            }
            case 1: {
                strRtn.assign("#06|001|User_offline");
                break;
            }
            default: {
                strRtn.assign("#06|002|Failure");
                break;
            }
        }
        write_s(confd, strRtn.c_str(), strRtn.size());
        break;
    }
    case 7: { // 用户读取离线消息
        readOfflineMessage(buf, confd, mysql);
        strRtn = "#07|Offline_Message_Finish";
        write_s(confd, strRtn.c_str(), strRtn.size());
        break;
    }
    case 8: { // 用户管理好友
        switch (atoi(buf.substr(4, 6).c_str())) {
            case 0: {
                switch (addFriend(buf, mysql)) {
                    case 0: {
                        strRtn.assign("#08|000|Success");
                        break;
                    }
                    case 1: {
                        strRtn.assign("#08|001|Already_Friend");
                        break;
                    }
                    case 2: {
                        strRtn.assign("#08|002|User_Not_Exist");
                        break;
                    }
                    default: { // 应该只有3
                        strRtn.assign("#08|003|Failure");
                        break;
                    }
                }
                break;
            }
            case 1: {
                switch(delFriend(buf, mysql)) {
                    case 0: {
                        strRtn.assign("#08|010|Success");
                        break;
                    }
                    case 1: {
                        strRtn.assign("#08|011|Not_Friend");
                        break;
                    }
                    default: { // 应该只有2
                        strRtn.assign("#08|012|Failure");
                        break;
                    }
                }
            }
            default:
                break;
        }
        write_s(confd, strRtn.c_str(), strRtn.size());
        break;
    }
    case 9: { // 服务端收客户端文件
        switch(receiveFile(buf)) {
            // case 0: {
            //     strRtn.assign("#09|000|Success");
            //     break;
            // }
            // default: { // 应该只有1
            //     strRtn.assign("#09|001|Failure");
            //     break;
            // }
        }
        // write_s(confd, strRtn.c_str(), strRtn.size());
        break;
    }
    case 10: {
        for(int i = 0;i < 50; i++) {
            strRtn = "Server: ";
            usleep(100 * 1000);
            printf("Test send: %d times\n", i);
            write_s(confd, strRtn.c_str(), strRtn.size());
        }
        break;
    }
    default:
        break;
    }



    printf("handMessage receive: %s...\n", buf.substr(0, (buf.size() < 51 ? buf.size()-1:50)).c_str());
    printf("handMessage send: %s...\n", strRtn.substr(0, (strRtn.size() < 51 ? strRtn.size()-1:50)).c_str());
    mysql_close(&mysql);
    return 0;
}

/**************************************************/
/*名称：handData
/*描述：队列缓存,解决黏包断包,减少读取等待
/*参数：buf格式%0000%%#00|000|xxxxxx...   
/*返回值：void
/***************************************************/
void handData(std::string &buf, const int &confd) {
    while(1) {
        if(buf.size() < 7)
            return;
        int len = atoi(buf.substr(1,4).c_str());
        if(buf.size() < 7 + len) {
            return;
        }

        // printf("%s\n", buf.substr(0, buf.size() > 20 ? 20:buf.size()-1).c_str());
        buf.erase(0, 7);
        std::string message = buf.substr(0, len);
        buf.erase(0, len);
        handMessage(message, confd);
    }
}


int main() {
    struct sockaddr_in sevaddr;
	memset(&sevaddr, 0, sizeof(sevaddr));
	sevaddr.sin_family = AF_INET;
	sevaddr.sin_port = htons(8899);
	sevaddr.sin_addr.s_addr = inet_addr("172.17.0.8");
	
	std::vector<int> clifd; // int clifd[1024] -> vector
	int confd = 0;
	int curi = 0;

	int maxfd = 0;
	fd_set allset, rset;
	FD_ZERO(&allset);

	int lisfd = socket(AF_INET, SOCK_STREAM, 0);

    int optval = 1;
	setsockopt(lisfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if(bind(lisfd, (struct sockaddr *)&sevaddr, sizeof(sevaddr)) != 0) {
		printf("bind error\n");
		return -1;
	}

	listen(lisfd, 100);

	FD_SET(lisfd, &allset);
	if(lisfd > maxfd)
		maxfd = lisfd;

	while(1) {
		rset = allset;
		int nready = select(maxfd+1, &rset ,NULL, NULL, NULL);
		if(nready <= 0) {
			printf("select error\n");
			return -1;
		}
		if(FD_ISSET(lisfd, &rset)) {
			int confd = accept(lisfd, NULL, NULL);
			if(confd == -1) {
				printf("accept error\n");
				return -1;
			}
			FD_SET(confd, &allset);
			if(confd > maxfd)
				maxfd = confd;
            
            write_s(confd, "#00|000|Success", 15);
            clifd.push_back(confd);

            if(confd < readBuf.size())
                readBuf[confd].clear();
            else {
                readBuf.resize(confd+1);
            }

		}
		for(int i = 0; i < clifd.size(); i++) {
			confd = clifd[i];
			if(FD_ISSET(confd, &rset)) {
				char buf[5001] = {0};
				if(read(confd, buf, 5000) == 0) { // 客户端断开连接
					FD_CLR(confd, &allset);
					clifd.erase(clifd.begin() + i);
                    users.delUser(confd);
                    i--;
                    if(confd < readBuf.size())
                        readBuf[confd].clear();
				}
                else {
                    std::string &userBuf = readBuf[confd];
                    userBuf.append(buf);
                    handData(userBuf, confd);
                }
				// handData(std::string(buf), confd);
            }
		}
        // ------------------------
        sendFile();
        // ------------------------
	}
	return 0;
}
