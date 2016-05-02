#include "client_thread.h"
#define SERVER_ADDRESS "127.0.0.1"

using namespace std;


int target,ret;
SOCKET cmdsock;
SOCKET datsock;
struct sockaddr_in sin1;
struct sockaddr_in sin2;
char buff[BIG_SIZE] = {0};
char fileName[SMALL_SIZE] = {0};
char hostName[SMALL_SIZE] = {0};


void initialize(char *cmdport,char *datport){
    /*-------------------create Socket--------------------*/
    struct timeval tv;
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    if ((cmdsock = socket(AF_INET,SOCK_STREAM,0)) < 0){
        printf("fail to initial cmdsocket!!!\n");
        exit(2);
    }
    if ((datsock = socket(AF_INET,SOCK_STREAM,0)) < 0){
        printf("fail to initial datsocket!!!\n");
        exit(2);
    }
    if(setsockopt(cmdsock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){
        printf("fail to config cmdsocket!!!\n");
        exit(2);
    }
    if(setsockopt(datsock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){
        printf("fail to config datsocket!!!\n");
        exit(2);
    }
    cout << "[STATE]client creates cmdsocket " << cmdsock << endl;
    cout << "[STATE]client creates datsocket " << datsock << endl;

    /*-----------------connect to the server--------------*/
    sin1.sin_family = AF_INET;
    sin1.sin_port = htons(atoi(cmdport));
    sin1.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    if ((connect(cmdsock,(struct sockaddr*)&sin1,sizeof(sin1))) < 0){
        cout << "[WARNING]client connecting cmdsocket fails!" << endl;
        exit(-1);
    }

    sin2.sin_family = AF_INET;
    sin2.sin_port = htons(atoi(datport));
    sin2.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    if ((connect(datsock,(struct sockaddr*)&sin2,sizeof(sin2))) < 0){
        cout << "[WARNING]client connecting datsocket fails!" << endl;
        exit(-1);
    }


    /*----------------------debug-------------------------*/
    cout << "[STATE]client: cmdIP   " << sin1.sin_addr.s_addr << endl;
    cout << "[STATE]client: datIP   " << sin2.sin_addr.s_addr << endl;
    cout << "[STATE]client: cmdport " << sin1.sin_port << endl;
    cout << "[STATE]client: datport " << sin2.sin_port << endl;
    cout << "[SUCCESS]client socket starts up!" << endl;
}

int main(int argc,char *argv[]){
    if (argc != 3){
        cout << "Please enter two client ports!" << endl;
        return -1;
    }
    initialize(argv[1],argv[2]);
    client_Thread *cThread = new client_Thread(cmdsock,datsock);
    cThread->run();
    return 0;
}
