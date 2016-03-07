#include <iostream>

extern "C"{
    #include "unp.h"
}

using namespace std;

void reply(int sockfd, SA *pcliaddr, socklen_t clilen){
    socklen_t len;
    char buffer[MAXLINE];

    while (TRUE){
        bzero(buffer, MAXLINE);
        len = clilen;
        cout << "waiting..." << endl;
        int ret = Recvfrom(sockfd, buffer, MAXLINE, 0, pcliaddr, &len);
        string message = string(buffer);
        message = message.substr(0, message.find('\n'));
        cout << "Receiving message:" << message << message.length() << endl;
        if (message == "bye"){
            break;
        }
        Sendto(sockfd, message.c_str(), ret, 0, pcliaddr, len);
    }
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    Bind(sockfd, (SA*) &servaddr, sizeof(servaddr));
    reply(sockfd, (SA*) &cliaddr, sizeof(cliaddr));

    return 0;
}