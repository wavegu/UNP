#include <iostream>
#include "query.h"

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
        Recvfrom(sockfd, buffer, MAXLINE, 0, pcliaddr, &len);
        RequestPackage package = *((RequestPackage*)buffer);
        ResponsePackage response_package;
        response_package.package_num = package.package_num;
        response_package.tot_package_num = package.tot_package_num;
        response_package.response_type = CHECK_REQUEST;
        strcpy(response_package.timestamp, package.timestamp);
        strcpy(response_package.content, package.content);
        Sendto(sockfd, (char*) &response_package, sizeof(response_package), 0, pcliaddr, clilen);
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