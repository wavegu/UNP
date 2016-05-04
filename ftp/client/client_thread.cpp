#include "client_thread.h"
#include "../server/const.h"
#define ENDING "ENDING"
using namespace std;

/*根据不同的reply字段,作出不同响应*/
int client_Thread::deal_with_reply(Package replyPackage, uint16_t ident){

    if (ntohs(replyPackage.package_type) != ptype::SERVER_CMD_PACKAGE){
        cout << "[ERROR]@client_thread::deal_with_reply package type error! " << replyPackage.package_type << endl;
        return rtn::PACKAGE_TYPE_ERROR;
    }

    string reply = (string)replyPackage.data;

    if (reply == "cmd"){
       return getCMDlist(ident);
    }

    if (reply == "currentDir"){
        return getcurrentDir(ident);
    }

    if (reply == "listDir"){
        return getDirlist(ident);
    }

    if (reply == "cd"){
        return changeDir(ident);
    }

    if (reply == "download"){
        return download(ident);
    }

    if (reply == "quit"){
        return quit(ident);
    }

    if (reply == "error"){
        cout << "[WARNING]wrong cmd,please try again!" << endl;
    }

    return rtn::UNEXPECTED_PACKAGE;
}

/*获取合法命令列表*/
int client_Thread::getCMDlist(uint16_t ident){
    Package package;
    int ret = recvPackage(datsock, &package, ident);
    if (ret != rtn::SUCCESS){
        cout << "[ERROR]@client_Thread::getCmdlist recvf cmd list fails! " << ret << endl;
        return ret;
    }
    cout << package.data << endl;
    return rtn::SUCCESS;
}

/*获取服务器当前目录列表*/
int client_Thread::getDirlist(uint16_t ident){
    Package package;
    int ret = recvPackage(datsock, &package, ident);
    if (ret != rtn::SUCCESS){
        cout << "[ERROR]@client_Thread::getDirlist recvf ls result fails! " << ret << endl;
        return ret;
    }
    cout << package.data << endl;
    return rtn::SUCCESS;
}

/*获取服务器当前工作目录*/
int client_Thread::getcurrentDir(uint16_t ident){
    Package package;
    int ret = recvPackage(datsock, &package, ident);
    if (ret != rtn::SUCCESS){
        cout << "[ERROR]@client_Thread::getcurrentDir recv pwd result fails! " << ret << endl;
        return ret;
    }
    cout << package.data << endl;
    return rtn::SUCCESS;
}

/*改变服务器当前工作目录*/
int client_Thread::changeDir(uint16_t ident){
    //发送新的文件名
    cout << "Please enter filename!" << endl;
    char buff[BIG_SIZE];
    cin >> buff;
    Package package;
    int ret = sendPackage(datsock, &package, buff, strlen(buff), ptype::CLIENT_DATA_PACKAGE);
    if (ret != rtn::SUCCESS){
        cout << "[ERROR]@client_Thread::changeDir send cd target fails! " << ret << endl;
        return ret;
    }
    cout << "[STATE]new dir sent to server" << endl;
    //接收服务器回复
    Package rpackage;
    ret = recvPackage(datsock, &rpackage, ident);
    if (ret != rtn::SUCCESS){
        cout << "[ERROR]@client_Thread::getcurrentDir recv pwd result fails! " << ret << endl;
        return ret;
    }
    cout << rpackage.data << endl;
    return rtn::SUCCESS;
}

/*从服务器下载文件*/
int client_Thread::download(uint16_t ident){
    //发送请求文件名
    cout << "[DOWNLOAD]Please enter filename" << endl;
    char filename[BIG_SIZE];
    cin >> filename;
    Package package;
    int ret = sendPackage(datsock, &package, filename, strlen(filename), ptype::CLIENT_DATA_PACKAGE);
    if (ret != rtn::SUCCESS){
        cout << "[ERROR]@client_Thread::download send target filename fails! " << ret << endl;
        return ret;
    }
    //接收文件存在确认
    Package ckPackage;
    ret = recvPackage(datsock, &ckPackage, ident);
    cout << "file recving checking " << ret << endl;
    cout << ckPackage.ident << " version=" << ckPackage.version << " ap=" << ckPackage.auth_protocol << " data=" << ckPackage.data << endl;

    if (ret != rtn::SUCCESS){
        cout << "[ERROR]@client_Thread::download recv filename existance fails! " << ret << endl;
        return ret;
    }
    string tem = (string)ckPackage.data;
    if (tem == "[SERVER]file not found"){
        cout << "[ERROR]@client_Thread::download file not found: " << filename << endl;
        return rtn::FILE_DONT_EXIST;
    }
    //接收文件
    int file = open(filename, (O_WRONLY | O_CREAT));
    Package filePackage;
    ret = recvPackage(datsock, &filePackage, ident);
    while(ret == rtn::SUCCESS){
        if (ntohs(filePackage.package_type) == ptype::SERVER_EOF_PACKAGE)
            break;
        write(file,filePackage.data,strlen(filePackage.data));
        cout << "writing " << filePackage.data << endl;
//        if (strlen(filePackage.data) < PACKAGE_DATA_SIZE) break;
        ret = recvPackage(datsock, &filePackage, ident);
    }
    if (ret != rtn::SUCCESS) {
        cout << "[ERROR]@client_Thread::download file fails: " << ret << endl;
        return ret;
    }
    close(file);
    cout << "[STATE]downloading file success" << endl;
    return rtn::SUCCESS;
}

/*客户端离线*/
int client_Thread::quit(uint16_t ident){
    cout << "[QUIT]Good Bye!" << endl;
    exit(1);
}

/*发送请求数据包*/
int client_Thread::sendPackage(SOCKET sock, Package *package, char *content, int slen, uint16_t package_type){
    cout << "sending " << content << endl;
    int plen = slen + PACKAGE_HEAD_SIZE;
    package->ident = htons(rand()|IDENT_RQ_BIT);
    package->version = htons(VERSION);
    package->auth_protocol = htons(0);
    package->package_type = htons(package_type);
    clean(package->data);
    strncpy(package->data, content, slen);
    package->checksum = 0;
    package->checksum = htons(calc_checksum((uint8_t*)package, plen));
    int ret = write(sock,package,plen);
    if (ret < 0) {
        cout << "[ERROR]@client_Thread::sendPackages " << ret << endl;
        return rtn::SEND_PACKAGE_ERROR;
    }
    return rtn::SUCCESS;
}

/*接受响应数据包*/
int client_Thread::recvPackage(SOCKET sock, Package *package, uint16_t ident) {
    clean(package->data);
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
    uint16_t recv_ident = ntohs(package->ident);
    if ((recv_ident & IDENT_RQ_BIT) || (recv_ident | IDENT_RQ_BIT) != ident){
        cout << "[ERROR]@client_Thread::recvPackage ident doesn't match" << endl;
        return rtn::PACKAGE_IDENT_INCONSISTENT;
    }
    // 检查checksum
    uint16_t checksum = ntohs(package->checksum);
    package->checksum = 0;
    if (checksum != calc_checksum((uint8_t*)package, ret)){
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
        ret = recvPackage(cmdsock, &replyPackage, ntohs(cmdPackage.ident));
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
        ret = deal_with_reply(replyPackage, ntohs(cmdPackage.ident));
        if (! ret){
            cout << "[ERROR]@client_Thread::run deal with server reply fails: " << replyPackage.data << endl;
            continue;
        }
    }
}
