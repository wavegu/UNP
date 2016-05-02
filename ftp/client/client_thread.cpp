#include "client_thread.h"
#include "../server/const.h"
#define ENDING "ENDING"
using namespace std;

/*根据不同的reply字段,作出不同响应*/
int client_Thread::deal_with_reply(string reply){

    if (reply == "cmd"){
       return getCMDlist();
    }

    if (reply == "currentDir"){
        return getcurrentDir();
    }

    if (reply == "listDir"){
        return getDirlist();
    }

    if (reply == "cd"){
        return changeDir();
    }

    if (reply == "download"){
        return download();
    }

    if (reply == "upload"){
        return upload();
    }

    if (reply == "quit"){
        return quit();
    }

    if (reply == "error"){
        cout << "[WARNING]wrong cmd,please try again!" << endl;
    }
    return 0;
}

/*获取合法命令列表*/
int client_Thread::getCMDlist(){
    char buff[BIG_SIZE];
    int ret;
    clean(buff);
    ret = read(datsock,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]read cmd fails!" << endl;
        return 0;
    }
    cout << buff << endl;
    return 1;
}

/*获取服务器当前目录列表*/
int client_Thread::getDirlist(){
    char buff[BIG_SIZE];
    int ret;
    clean(buff);
    ret = read(datsock,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]read dir list fails!" << endl;
        return 0;
    }
    cout << "dir list:\n" << buff << endl;
    return 1;
}

/*获取服务器当前工作目录*/
int client_Thread::getcurrentDir(){
    char buff[BIG_SIZE];
    int ret;
    clean(buff);
    ret = read(datsock,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]read current dir fails!" << endl;
        return 0;
    }
    cout << buff << endl;
    return 1;
}

/*改变服务器当前工作目录*/
int client_Thread::changeDir(){
    //发送新的文件名
    cout << "Please enter filename!" << endl;
    char buff[BIG_SIZE];
    cin >> buff;
    int ret = write(datsock,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]send new dir fails!" << endl;
        return 0;
    }
    cout << "[STATE]new dir sent to server" << endl;
    clean(buff);
    //接收服务器回复
    ret = read(datsock,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]cd recv server reply fails!" << endl;
        return 0;
    }
    cout << buff << endl;
    return 1;
}

/*上传文件到服务器*/
int client_Thread::upload(){
    //发送文件名
    cout << "[UPLOAD]Please enter filename" << endl;
    char buff[BIG_SIZE];
    cin >> buff;
    int ret = write(datsock,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]sending filename fails!" << endl;
        return 0;
    }
    //发送文件
    int file = open(buff, O_RDONLY);
    clean(buff);
    int len;
    while(len = read(file,buff,BIG_SIZE)){
        write(datsock,buff,BIG_SIZE);
        if (len < BIG_SIZE)  break;
    }
    //发送终止符
    string tem = ENDING;
    clean(buff);
    strcpy(buff,tem.c_str());
    write(datsock,buff,BIG_SIZE);
    cout << "[STATE]sending file over" << endl;
    close(file);

    return 1;
}

/*从服务器下载文件*/
int client_Thread::download(){
    //发送请求文件名
    cout << "[DOWNLOAD]Please enter filename" << endl;
    char filename[BIG_SIZE];
    cin >> filename;
    int ret = write(datsock,filename,sizeof(filename));
    if (ret < 0){
        cout << "[WARNING]sending filename fails!" << endl;
        return 0;
    }
    //接收文件存在确认
    char buff[BIG_SIZE];
    ret = read(datsock,buff,sizeof(buff));
    if (ret < 0){
        cout << "[WARNING]recving file confirm fails!" << endl;
        return 0;
    }
    string tem = (string)buff;
    cout << tem << endl;
    if (tem == "[SERVER]file not found"){
        return 0;
    }
    //接收文件
    clean(buff);
    int file = open(filename, (O_WRONLY | O_CREAT));
    int len;
    while(len = read(datsock,buff,sizeof(buff))){
        tem = (string)buff;
        if (tem == ENDING)  break;
        write(file,buff,BIG_SIZE);
        if (len < BIG_SIZE) break;
    }
    close(file);
    cout << "[STATE]downloading file success" << endl;
    return 1;
}

/*客户端离线*/
int client_Thread::quit(){
    cout << "[QUIT]Good Bye!" << endl;
    exit(1);
}

/*发送请求数据包*/
int client_Thread::sendPackage(SOCKET sock, Package *package, char *content, int slen, uint16_t package_type){
    cout << "sending " << content << endl;
    package->ident = htons(rand()|IDENT_RQ_BIT);
    package->version = htons(0);
    package->auth_protocol = htons(0);
    package->package_type = htons(package_type);
    package->checksum = htons(calc_checksum((uint8_t*)package, slen));
    strncpy(package->data, content, slen);
    int plen = slen + PACKAGE_HEAD_SIZE;
    int ret = write(sock,package,plen);
    if (ret < 0) {
        cout << "[ERROR]@client_Thread::sendPackages " << ret << endl;
        return rtn::SEND_PACKAGE_ERROR;
    }
    return rtn::SUCCESS;
}

/*接受响应数据包*/
int client_Thread::recvPackage(SOCKET sock, Package *package, uint16_t ident) {
    int ret = read(sock, package, sizeof(*package));
    // 检查接收是否成功
    if (ret < 0){
        if(errno == EAGAIN || errno == EWOULDBLOCK){
            cout << "[ERROR]@client_Thread::recvPackage TIMEOUT" << endl;
            return rtn::RECV_TIMEOUT;
        }
        cout << "[ERROR]@client_Thread::recvPackage " << ret << endl;
        return rtn::RECV_PACKAGE_ERROR;
    }
    // 检查数据包是否完整
    if (ret < PACKAGE_HEAD_SIZE){
        cout << "[ERROR]@client_Thread::recvPackage package too small" << endl;
        return rtn::PACKAGE_INCOMPLETE;
    }
    // 检查ident
    if (ntohs(package->ident) != ident){
        cout << "[ERROR]@client_Thread::recvPackage ident doesn't match" << endl;
        return rtn::PACKAGE_IDENT_INCONSISTENT;
    }
    // 检查checksum
    if (ntohs(package->checksum) != calc_checksum((uint8_t*)package, ret)){
        cout << "[ERROR]@client_Thread::recvPackage cksum doesn't match" << endl;
        return rtn::PACKAGE_CKSUM_INCONSISTENT;
    }
    return rtn::SUCCESS;
}


/*计算校验和*/
uint16_t client_Thread::calc_checksum(uint8_t* data, int len)
{
    uint16_t sum = 0;
    int i;
    for(i=0; i<len; i++){
        sum += data[i];
    }
    return sum;
}


/*等待输入指令,发送指令,等待回复,把回复丢给deal_with_reply函数*/
void client_Thread::run(){
    int ret = 0;
    while (true){
        //向服务器发送指令
        char buff[BIG_SIZE];
        clean(buff);
        cout << "Please send your command:" << endl;
        cin >> buff;
        if (strlen(buff) > PACKAGE_DATA_SIZE - 1) {
            cout << "[ERROR]@client_Thread::run cmd too long, please type again!" << endl;
            continue;
        }
        Package cmdPackage;
        ret = sendPackage(cmdsock, &cmdPackage, buff, strlen(buff), ptype::CLIENT_CMD_PACKAGE);
        if (ret != rtn::SUCCESS){
            cout << "[ERROR]@client_Thread::run client sending cmd fails! " << ret << endl;
            continue;
        }
        cout << buff << " successfully sent!" << endl;

        //接收服务器回复指令
        Package replyPackage;
        ret = recvPackage(cmdsock, &replyPackage, cmdPackage.ident);
        if (ret != rtn::SUCCESS){
            cout << "[ERROR]@client_Thread::run get server reply fails! " << ret << endl;
            continue;
        }
        if (htons(replyPackage.package_type) != ptype::SERVER_CMD_PACKAGE){
            cout << "[ERROR]@client_Thread::run server reply not cmd " << htons(replyPackage.package_type) << endl;
            continue;
        }
        cout << "[OK]@client_Thread::run get server reply: " << endl << replyPackage.data << endl;
        //处理服务器回复
        ret = deal_with_reply((string)replyPackage.data);
        if (! ret){
            cout << "[ERROR]@client_Thread::run deal with server reply fails: " << replyPackage.data << endl;
            continue;
        }
    }
}
