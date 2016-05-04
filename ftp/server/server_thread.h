#ifndef SERVER_THREAD_H
#define SERVER_THREAD_H

#include <QThread>
#include <stdio.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <cstdlib>
#include <cerrno>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include "package.h"


#define BIGSIZE 1024
#define SMALLSIZE 50
#define ENDING "Eric-----ending of file-----"
#define clean(x) (memset(x,0,sizeof(x)))

typedef int SOCKET;

class server_Thread:public QThread{
private:
    SOCKET cmdvisitor;
    SOCKET datvisitor;
    int deal_with_cmd(Package package);
    /*标准操作流程:
    * [1]发送一个reply
    * [2]do sth
    * reply类型:
    * [1]cmd: send cmd list
    * [2]currentDir: send current dir
    */
    int sendFile(uint16_t ident);
    int listCmd(uint16_t ident);
    int showDir(uint16_t ident);
    int listDir(uint16_t ident);
    int changeDir(uint16_t ident);
    int clientQuit(uint16_t ident);

    int recvPackage(SOCKET sock, Package *package);
    int sendPackage(SOCKET sock, Package *package, char *content, int slen, uint16_t package_type, uint16_t ident);
    bool fileExist(char *filename);
    uint16_t calc_checksum(uint8_t* data, int len);
public:
    server_Thread(SOCKET cs,SOCKET ds);
    void run();
};

#endif // SERVER_THREAD


