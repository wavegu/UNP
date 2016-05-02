#include <iostream>
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
#include <fstream>
#include <unistd.h>
#include "server_thread.h"

#define LOCAL_IP "127.0.0.1"
#define TIMEOUT 1

using namespace std;

SOCKET cmdSock;
SOCKET datSock;
struct sockaddr_in sin1;
struct sockaddr_in sin2;

void initialize(char *port1,char *port2){
    /*----------------------create cmdSocket----------------------*/
    struct timeval tv;
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    if ((cmdSock = socket(AF_INET,SOCK_STREAM,0)) < 0){
        cout << "[WARNING]server cmdSocket fails to startup!" << endl;
        exit(2);
    }
    if ((datSock = socket(AF_INET,SOCK_STREAM,0)) < 0){
        cout << "[WARNING]server datSocket fails to startup!" << endl;
        exit(2);
    }
    if(setsockopt(cmdSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){
        printf("fail to config cmdsocket!!!\n");
        exit(2);
    }
    if(setsockopt(datSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){
        printf("fail to config datsocket!!!\n");
        exit(2);
    }
    /*----------------------------bind-------------------------*/
    sin1.sin_port = htons(atoi(port1));
    sin1.sin_family = AF_INET;
    sin1.sin_addr.s_addr = inet_addr(LOCAL_IP);
    if ((bind(cmdSock,(struct sockaddr*)&sin1,sizeof(sin1))) < 0){
        cout << "[WARNING]cmdSock binding fails!" << endl;
        exit(2);
    }

    sin2.sin_port = htons(atoi(port2));
    sin2.sin_family = AF_INET;
    sin2.sin_addr.s_addr = inet_addr(LOCAL_IP);
    if ((bind(datSock,(struct sockaddr*)&sin2,sizeof(sin2))) < 0){
        cout << "[WARNING]cmdSock binding fails!" << endl;
        exit(2);
    }

    /*--------------------------listen-------------------------*/
    if ((listen(cmdSock,15)) < 0){
        cout << "[WARNING]listen fails!" << endl;
        exit(-1);
    }
    if ((listen(datSock,15)) < 0){
        cout << "[WARNING]listen fails!" << endl;
        exit(-1);
    }
    cout << "[STATE]server: IP     " << sin1.sin_addr.s_addr << endl;
    cout << "[STATE]server: cmdPort   " << sin1.sin_port << endl;
    cout << "[STATE]server: datPort   " << sin2.sin_port << endl;
    cout << "[STATE]server: cmdSocket " << cmdSock << endl;
    cout << "[STATE]server: datSocket " << datSock << endl;
    cout << "[STATE]server starts up!" << endl;
}

void deal_with_request(){
    while(1){
        cout << "waiting for visitor ..." << endl;
        SOCKET cmdVisitor;
        SOCKET datVisitor;
        cout << "[STATE]server waiting for client request......" << endl;
        /*---------------------accept-----------------------*/
        if ((cmdVisitor = accept(cmdSock,NULL,NULL)) < 0){
            cout << "[WARNING]cmd acception fails!" << endl;
            exit(-1);
        }
        if ((datVisitor = accept(datSock,NULL,NULL)) < 0){
            cout << "[WARNING]dat acception fails!" << endl;
            exit(-1);
        }
        cout << "[STATE]client connected:" << endl <<
                "[cmdSock] " << cmdVisitor << endl <<
                "[datSock] " << datVisitor << endl;

        server_Thread *visitorThread = new server_Thread(cmdVisitor,datVisitor);
        visitorThread->start();
    }
}

int main(int argc,char **argv){
    if (argc != 3){
        cout << "Please input your two ports!" << endl;
        return 0;
    }
    initialize(argv[1],argv[2]);
    deal_with_request();
    return 0;
}
