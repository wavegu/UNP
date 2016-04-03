#include <iostream>
#include "operator.h"

extern "C"{
    #include "unp.h"
}

using namespace std;


int main() {

    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    Bind(sockfd, (SA*) &servaddr, sizeof(servaddr));

    srand((unsigned int)time(NULL));
    Operator op = Operator(sockfd, (SA*) &cliaddr, sizeof(cliaddr));
    op.run();

    return 0;
}