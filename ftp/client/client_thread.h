#ifndef CLIENT_THREAD_H
#define CLIENT_THREAD_H

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
#include <unistd.h>
#include "../server/package.h"

#define BIG_SIZE 1024
#define SMALL_SIZE 50
#define clean(x) (memset(x,0,sizeof(x)))

typedef int SOCKET;

class client_Thread{
private:
    SOCKET cmdsock;
    SOCKET datsock;
public:
    client_Thread(SOCKET cs = 0,SOCKET ds = 0):cmdsock(cs),datsock(ds){}
    int deal_with_reply(std::string);
    int getCMDlist();
    int getDirlist();
    int getcurrentDir();
    int changeDir();
    int upload();
    int download();
    int quit();
    int recvPackage(SOCKET sock, Package *package, uint16_t ident);
    int sendPackage(SOCKET sock, Package *package, char *content, int slen, uint16_t package_type);
    uint16_t calc_checksum(uint8_t* data, int len);
    void run();
};
#endif // CLIENT_THREAD_H
