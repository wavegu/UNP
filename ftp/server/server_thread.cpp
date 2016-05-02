#include "const.h"
#include "server_thread.h"

using namespace std;

bool server_Thread::fileExist(char *filename){
    FILE *f = fopen(filename,"r");
    if (f == NULL){
        return 0;
    }
    fclose(f);
    return 1;
}

server_Thread::server_Thread(SOCKET cs,SOCKET ds):cmdvisitor(cs),datvisitor(ds){}

/*服务器发送文件*/
int server_Thread::sendFile(){
    //发送响应类型
    char buff[BIGSIZE] = "download";
    int ret = write(cmdvisitor,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]sending reply download fails" << endl;
        return 0;
    }
    clean(buff);
    //接收文件名
    ret = read(datvisitor,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]recv filename fails" << endl;
        return 0;
    }
    //发送文件存在确认
    if (!fileExist(buff)){
        string tem = "[SERVER]file not found";
        strcpy(buff,tem.c_str());
        write(datvisitor,buff,sizeof(buff));
        return 0;
    }
    char reply[BIGSIZE];
    string tem = "[SERVER]file ready!";
    strcpy(reply,tem.c_str());
    ret = write(datvisitor,reply,sizeof(reply));
    if (ret < 0){
        cout << "[WARNING]file confirm sending fails" << endl;
        return 0;
    }
    //发送文件
    int file = open(buff, O_RDONLY);
    clean(buff);
    int len;
    while(len = read(file,buff,BIGSIZE)){
        write(datvisitor,buff,BIGSIZE);
        if (len < BIGSIZE)  break;
    }
    //发送终止符
    tem = ENDING;
    clean(buff);
    strcpy(buff,tem.c_str());
    write(datvisitor,buff,BIGSIZE);
    cout << "[STATE]sending file over" << endl;
    close(file);
    return 1;
}

/*服务器接收文件*/
int server_Thread::getFile(){
    //发送响应类型
    char buff[BIGSIZE] = "upload";
    int ret = write(cmdvisitor,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]sending reply upload fails" << endl;
        return 0;
    }
    //接收文件名
    char filename[BIGSIZE];
    ret = read(datvisitor,filename,sizeof(filename));
    if (ret < 0){
        cout << "[WARNING]recv filename fails" << endl;
        return 0;
    }
    //接收文件
    clean(buff);
    int file = open(filename, (O_WRONLY | O_CREAT));
    int len;
    string tem;
    while(len = read(datvisitor,buff,sizeof(buff))){
        tem = (string)buff;
        if (tem == ENDING)  break;
        write(file,buff,BIGSIZE);
        if (len < BIGSIZE)  break;
    }
    close(file);
    cout << "[STATE]uploading file success" << endl;
    return 1;
}

/*列出合法命令*/
int server_Thread::listCmd(){
    //发送响应类型
    char buff[BIGSIZE] = "cmd";
    int ret = write(cmdvisitor,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]sending reply cmd fails" << endl;
        return 0;
    }
    clean(buff);
    //发送合法命令
    string cmdlist = "\nCMD list:\n\
[1]get : request a file from server\n\
[2]put : send a file to server\n\
[3]pwd : request current directory of server\n\
[4]dir : list current directory of server\n\
[5]cd  : change current dir\n\
[6]quit: quit\n";
    strcpy(buff,cmdlist.c_str());
    ret = write(datvisitor,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]sending command list FAILS" << endl;
        return 0;
    }
    cout << "[STATE]cmd list sent to client" << endl;
    return 1;
}


/*列出服务器当前目录*/
int server_Thread::listDir(){
    char buff[BIGSIZE] = "listDir";
    //发送响应类型
    int ret = write(cmdvisitor,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]sending reply listDir fails" << endl;
        return 0;
    }
    clean(buff);
    //发送目录列表
    system("ls>/tmp/dirlist.txt");
    int filehandle = open("/tmp/dirlist.txt",O_RDONLY);
    read(filehandle,buff,sizeof(buff));
    ret = write(datvisitor,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]sending dirlist fails!" << endl;
        return 0;
    }
    cout << "[STATE]dir list sent to client!" << endl;
    close(filehandle);
    system("rm -f /tmp/dirlist.txt");
    return 1;
}

/*显示服务器当前工作目录*/
int server_Thread::showDir(){
    char buff[BIGSIZE] = "currentDir";
    //发送响应类型
    int ret = write(cmdvisitor,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]sending reply currentDir fails" << endl;
        return 0;
    }
    clean(buff);
    //发送工作目录
    getcwd(buff,BIGSIZE);
    ret = write(datvisitor,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]sending current dir fails" << endl;
        return 0;
    }
    cout << "[STATE]current dir sent to client" << endl;
    return 1;
}

/*改变当前工作目录*/
int server_Thread::changeDir(){
    char buff[BIGSIZE] = "cd";
    //发送响应类型
    int ret = write(cmdvisitor,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]sending reply cd fails" << endl;
        return 0;
    }
    clean(buff);
    //改变工作目录
    ret = read(datvisitor,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]recving new dir fails" << endl;
        return 0;
    }
    if (! fileExist(buff)){
        string tem = "[SERVER]file " + (string)buff + " not found\n";
        clean(buff);
        strcpy(buff,tem.c_str());
        write(datvisitor,buff,sizeof(buff));
        return 0;
    }
    chdir(buff);
    clean(buff);
    //发送当前目录
    getcwd(buff,BIGSIZE);
    write(datvisitor,buff,sizeof(buff));
    return 1;
}

/*客户端离开*/
int server_Thread::clientQuit(){
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
int server_Thread::deal_with_cmd(string buff){
    if (buff == "get"){
        if (! sendFile()){
            cout << "[WARNING]sending file fails" << endl;
            return 0;
        }
        return 1;
    }
    if (buff == "put"){
        if (! getFile()){
            cout << "[WARNING]getting file fails" << endl;
            return 0;
        }
        return 1;
    }
    if (buff == "pwd"){
        if (! showDir()){
            cout << "[WARNING]showing current dir fails" << endl;
            return 0;
        }
        return 1;
    }
    if (buff == "dir"){
        if (! listDir()){
            cout << "[WARNING]listing dir fails" << endl;
            return 0;
        }
        return 1;
    }
    if (buff == "cd" ){
        if (! changeDir()){
            cout << "[WARNING]changing dir fails" << endl;
            return 0;
        }
        return 1;
    }
    if (buff == "?" || buff == "help"){
        if (! listCmd()){
            cout << "[WARNING]listing cmd fails!" << endl;
            return 0;
        }
        return 1;
    }
    if (buff == "quit"){
        if (! clientQuit()){
            cout << "[WARNING]quiting cmd fails!" << endl;
            return 0;
        }
        return 1;
    }

    buff = "Wrong CMD:" + buff;
    write(cmdvisitor,buff.c_str(),sizeof(buff.c_str()));
    return 0;
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
        int len = read(cmdvisitor,&clientCmdPackage,sizeof(clientCmdPackage));

        if (len < 0){
            cout << "[ERROR]@server_Thread::run server reading cmd fails!" << endl;
            retry++;
            continue;
        }

        if (len < PACKAGE_HEAD_SIZE){
            cout << "[ERROR]@server_Thread::run cmd package too small!" << endl;
            retry++;
            continue;
        }

        if (ntohs(clientCmdPackage.package_type) != ptype::CLIENT_CMD_PACKAGE){
            cout << "[ERROR]@server_Thread::run package_type error!" << ntohs(clientCmdPackage.package_type) << endl;
            retry++;
            continue;
        }

        retry = 0;

        len = deal_with_cmd((string)clientCmdPackage.data);
        if (!len)   continue;
    }
    close(cmdvisitor);
    close(datvisitor);
}


