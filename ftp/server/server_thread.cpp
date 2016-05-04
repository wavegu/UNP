#include "const.h"
#include "server_thread.h"

using namespace std;


server_Thread::server_Thread(SOCKET cs,SOCKET ds):cmdvisitor(cs),datvisitor(ds){}


bool server_Thread::fileExist(char *filename){
    FILE *f = fopen(filename,"r");
    if (f == NULL){
        return 0;
    }
    fclose(f);
    return 1;
}


uint16_t server_Thread::calc_checksum(uint8_t* data, int len){    
    uint16_t sum = 0;
    int i;
    for(i=0; i<len; i++){
        sum += data[i];
    }
    return sum;
}


/*服务器发送文件*/
int server_Thread::sendFile(uint16_t ident){
    //发送响应类型
    char buff[BIGSIZE] = "download";
    Package package;
    int ret = sendPackage(cmdvisitor, &package, buff, strlen(buff), ptype::SERVER_CMD_PACKAGE, ident);
    if (ret != rtn::SUCCESS) {
        cout << "[ERROR]@server_Thread::sendFile send cmd download fails! " << ret << endl;
        return ret;
    }
    //接收文件名
    ret = recvPackage(datvisitor, &package);
    if (ret != rtn::SUCCESS) {
        cout << "[ERROR]@server_Thread::sendFile recv target filename fails! " << ret << endl;
        return ret;
    }
    //发送文件存在确认
    clean(buff);
    if (!fileExist(package.data)){
        string tem = "[SERVER]file not found";
        strcpy(buff,tem.c_str());
        ret = sendPackage(datvisitor, &package, buff, strlen(buff), ptype::SERVER_DATA_PACKAGE, ident);
        if (ret != rtn::SUCCESS) {
            cout << "[ERROR]@server_Thread::sendFile send file not exist fails! " << ret << endl;
            return ret;
        }
    }
    char reply[BIGSIZE];
    string tem = "[SERVER]file ready!";
    strcpy(reply,tem.c_str());
    ret = sendPackage(datvisitor, &package, reply, strlen(reply), ptype::SERVER_DATA_PACKAGE, ident);
    if (ret != rtn::SUCCESS) {
        cout << "[ERROR]@server_Thread::sendFile send file ready fails! " << ret << endl;
        return ret;
    }
    //发送文件
    int file = open(buff, O_RDONLY);
    clean(buff);
    int len;
    while(len = read(file,buff,BIGSIZE)){
        ret = sendPackage(datvisitor, &package, buff, PACKAGE_DATA_SIZE, ptype::SERVER_DATA_PACKAGE, ident);
        if (ret != rtn::SUCCESS) {
            close(file);
            cout << "[ERROR]@server_Thread::sendFile when sending file! " << ret << endl;
            return ret;
        }
        if (len < BIGSIZE)  break;
    }
    //发送终止包
    ret = sendPackage(datvisitor, &package, buff, strlen(buff), ptype::SERVER_EOF_PACKAGE, ident);
    write(datvisitor,buff,BIGSIZE);
    if (ret != rtn::SUCCESS) {
        close(file);
        cout << "[ERROR]@server_Thread::sendFile when sending EOF! " << ret << endl;
        close(file);
        return ret;
    }
    cout << "[STATE]sending file over" << endl;
    close(file);
    return rtn::SUCCESS;
}


/*列出合法命令*/
int server_Thread::listCmd(uint16_t ident){
    //发送响应类型
    char buff[BIGSIZE] = "cmd";
    Package package;
    int ret = sendPackage(cmdvisitor, &package, buff, strlen(buff), ptype::SERVER_CMD_PACKAGE, ident);

    if (ret != rtn::SUCCESS) {
        cout << "[ERROR]@server_Thread::listCmd send cmd listCmd fails! " << ret << endl;
        return ret;
    }

    //发送合法命令
    clean(buff);
    string cmdlist = "\nCMD list:\n\
[1]get : request a file from server\n\
[2]put : send a file to server\n\
[3]pwd : request current directory of server\n\
[4]ls  : list current directory of server\n\
[5]cd  : change current dir\n\
[6]quit: quit\n";
    strcpy(buff,cmdlist.c_str());
    Package datPackage;
    ret = sendPackage(datvisitor, &datPackage, buff, strlen(buff), ptype::SERVER_DATA_PACKAGE, ident);
    if (ret != rtn::SUCCESS){
        cout << "[ERROR]@server_Thread::listCmd send cmd list fails! " << ret << endl;
        return ret;
    }
    cout << "[STATE]cmd list sent to client" << endl;
    return rtn::SUCCESS;
}


/*列出服务器当前目录*/
int server_Thread::listDir(uint16_t ident){
    char buff[BIGSIZE] = "listDir";
    //发送响应类型
    Package package;
    int ret = sendPackage(cmdvisitor, &package, buff, strlen(buff), ptype::SERVER_CMD_PACKAGE, ident);
    if (ret != rtn::SUCCESS) {
        cout << "[ERROR]@server_Thread::listDir send cmd ls fails! " << ret << endl;
        return ret;
    }

    //发送目录列表
    clean(buff);
    system("ls>/tmp/dirlist.txt");
    int filehandle = open("/tmp/dirlist.txt",O_RDONLY);
    read(filehandle,buff,sizeof(buff));
    close(filehandle);
    system("rm -f /tmp/dirlist.txt");
    Package datPackage;
    ret = sendPackage(datvisitor, &datPackage, buff, strlen(buff), ptype::SERVER_DATA_PACKAGE, ident);
    if (ret != rtn::SUCCESS){
        cout << "[ERROR]@server_Thread::listDir send cmd listDir fails! " << ret << endl;
        return ret;
    }
    cout << "[STATE]dir list sent to client!" << endl;
    return rtn::SUCCESS;
}

/*显示服务器当前工作目录*/
int server_Thread::showDir(uint16_t ident){
    char buff[BIGSIZE] = "currentDir";
    //发送响应类型
    Package package;
    int ret = sendPackage(cmdvisitor, &package, buff, strlen(buff), ptype::SERVER_CMD_PACKAGE, ident);

    if (ret != rtn::SUCCESS) {
        cout << "[ERROR]@server_Thread::showDir send cmd showDir fails! " << ret << endl;
        return ret;
    }

    //发送工作目录
    clean(buff);
    getcwd(buff,BIGSIZE);
    Package datPackage;
    ret = sendPackage(datvisitor, &datPackage, buff, strlen(buff), ptype::SERVER_DATA_PACKAGE, ident);
    if (ret != rtn::SUCCESS){
        cout << "[ERROR]@server_Thread::listDir send cmd listDir fails! " << ret << endl;
        return ret;
    }

    cout << "[STATE]current dir sent to client" << endl;
    return 1;
}

/*改变当前工作目录*/
int server_Thread::changeDir(uint16_t ident){
    char buff[BIGSIZE] = "cd";
    //发送响应类型
    Package package;
    int ret = sendPackage(cmdvisitor, &package, buff, strlen(buff), ptype::SERVER_CMD_PACKAGE, ident);
    if (ret != rtn::SUCCESS) {
        cout << "[ERROR]@server_Thread::changeDir send cmd cd fails! " << ret << endl;
        return ret;
    }

    //改变工作目录
    clean(buff);
    Package rpackage;
    ret = recvPackage(datvisitor, &rpackage);
    if (ret != rtn::SUCCESS) {
        cout << "[ERROR]@server_Thread::changeDir recv cd target fails! " << ret << endl;
        return ret;
    }
    if (! fileExist(rpackage.data)){
        string tem = "[SERVER]file " + (string)rpackage.data + " not found\n";
        strncpy(buff, tem.c_str(), strlen(tem.c_str()));
        ret = sendPackage(datvisitor, &package, buff, strlen(buff), ptype::SERVER_DATA_PACKAGE, ident);
        if (ret != rtn::SUCCESS) {
            cout << "[ERROR]@server_Thread::changeDir send file not found fails! " << ret << endl;
            return ret;
        }
    }
    chdir(rpackage.data);
    //发送当前目录
    getcwd(rpackage.data,BIGSIZE);
    ret = sendPackage(datvisitor, &package, rpackage.data, strlen(rpackage.data), ptype::SERVER_DATA_PACKAGE, ident);
    if (ret != rtn::SUCCESS) {
        cout << "[ERROR]@server_Thread::changeDir send current dir fails! " << ret << endl;
        return ret;
    }
    return rtn::SUCCESS;
}

/*客户端离开*/
int server_Thread::clientQuit(uint16_t ident){
    //发送响应类型
    char buff[BIGSIZE] = "quit";
    int ret = write(cmdvisitor,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]client quitting fails" << endl;
        return 0;
    }
    cout << "[STATE]client quits!" << endl;
    return 1;
}

/*判断命令类型,送入对应函数*/
int server_Thread::deal_with_cmd(Package cmdPackage){

    string cmd = (string)cmdPackage.data;
    uint16_t ident = ntohs(cmdPackage.ident);

    if (cmd == "get"){
        if (! sendFile(ident)){
            cout << "[WARNING]sending file fails" << endl;
            return 0;
        }
        return 1;
    }
    if (cmd == "pwd"){
        if (! showDir(ident)){
            cout << "[WARNING]showing current dir fails" << endl;
            return 0;
        }
        return 1;
    }
    if (cmd == "ls"){
        if (! listDir(ident)){
            cout << "[WARNING]listing dir fails" << endl;
            return 0;
        }
        return 1;
    }
    if (cmd == "cd" ){
        if (! changeDir(ident)){
            cout << "[WARNING]changing dir fails" << endl;
            return 0;
        }
        return 1;
    }
    if (cmd == "?" || cmd == "help"){
        if (! listCmd(ident)){
            cout << "[WARNING]listing cmd fails!" << endl;
            return 0;
        }
        return 1;
    }
    if (cmd == "quit"){
        if (! clientQuit(ident)){
            cout << "[WARNING]quiting cmd fails!" << endl;
            return 0;
        }
        return 1;
    }

    cmd = "Wrong CMD:" + cmd;
    write(cmdvisitor,cmd.c_str(),sizeof(cmd.c_str()));
    return 0;
}


/*发送请求数据包*/
int server_Thread::sendPackage(SOCKET sock, Package *package, char *content, int slen, uint16_t package_type, uint16_t ident){
    cout << "sending " << content << endl;
    int plen = slen + PACKAGE_HEAD_SIZE;
    package->ident = htons(ident ^ IDENT_RQ_BIT);
    package->version = htons(VERSION);
    package->auth_protocol = htons(0);
    package->package_type = htons(package_type);
    package->checksum = 0;
    clean(package->data);
    strncpy(package->data, content, slen);
    package->checksum = htons(calc_checksum((uint8_t*)package, plen));
    int ret = write(sock,package,plen);
    if (ret < 0) {
        cout << "[ERROR]@client_Thread::sendPackages " << ret << endl;
        return rtn::SEND_PACKAGE_ERROR;
    }
    return rtn::SUCCESS;
}

/*接受响应数据包*/
int server_Thread::recvPackage(SOCKET sock, Package *package) {
    clean(package->data);
    int ret = read(sock, package, sizeof(*package));
    // 检查接收是否成功
    if (ret < 0){
        if(errno == EAGAIN || errno == EWOULDBLOCK){
            cout << "[ERROR]@server_Thread::recvPackage TIMEOUT" << endl;
            return rtn::RECV_TIMEOUT;
        }
        cout << "[ERROR]@server_Thread::recvPackage " << ret << endl;
        return rtn::RECV_PACKAGE_ERROR;
    }
    // 检查数据包是否完整
    if (ret < PACKAGE_HEAD_SIZE){
        cout << "[ERROR]@server_Thread::recvPackage package too small" << endl;
        return rtn::PACKAGE_INCOMPLETE;
    }
    // 检查ident
    uint16_t recv_ident = ntohs(package->ident);
    if (!(recv_ident & IDENT_RQ_BIT)){
        cout << "[ERROR]@server_Thread::recvPackage no package ident" << endl;
        return rtn::PACKAGE_IDENT_INCONSISTENT;
    }
    // 检查checksum
    cout << "checking " << ret << endl;
    cout << package->ident << " version=" << package->version << " ap=" << package->auth_protocol << " data=" << package->data << endl;
    uint16_t checksum = ntohs(package->checksum);
    package->checksum = 0;
    if (checksum != calc_checksum((uint8_t*)package, ret)){
        cout << "[ERROR]@server_Thread::recvPackage cksum doesn't match" << endl;
        return rtn::PACKAGE_CKSUM_INCONSISTENT;
    }
    return rtn::SUCCESS;
}

void server_Thread::run(){

    sockaddr_in cmdAddr;
    sockaddr_in datAddr;
    socklen_t cmdlen = sizeof(cmdAddr);
    socklen_t datlen = sizeof(datAddr);
    getpeername(cmdvisitor, (sockaddr*)&cmdAddr, &cmdlen);
    getpeername(datvisitor, (sockaddr*)&datAddr, &datlen);

    /*---------------wait for command--------------*/
    int retry = 0;
    while (true){

        if (retry > 6) {
            cout << "[ERROR]@server_Thread::run Retry too much. Thread quits." << endl;
            break;
        }

        Package clientCmdPackage;
//        int len = read(cmdvisitor,&clientCmdPackage,sizeof(clientCmdPackage));

//        if (len < 0){
//            cout << "[ERROR]@server_Thread::run server reading cmd fails!" << endl;
//            retry++;
//            continue;
//        }

//        if (len < PACKAGE_HEAD_SIZE){
//            cout << "[ERROR]@server_Thread::run cmd package too small!" << endl;
//            retry++;
//            continue;
//        }

        int ret = recvPackage(cmdvisitor, &clientCmdPackage);
        if (ret != rtn::SUCCESS){
            retry++;
            cout << "[ERROR]@server_Thread::run recv client cmd error! " << ret << endl;
            continue;
        }

        if (ntohs(clientCmdPackage.package_type) != ptype::CLIENT_CMD_PACKAGE){
            cout << "[ERROR]@server_Thread::run package_type error!" << ntohs(clientCmdPackage.package_type) << endl;
            retry++;
            continue;
        }

        retry = 0;

        int len = deal_with_cmd(clientCmdPackage);
        if (!len)   continue;
    }
    close(cmdvisitor);
    close(datvisitor);
}


