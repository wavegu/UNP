#include <iostream>
#include "operator.h"

extern "C"{
    #include "unp.h"
}

using namespace std;


int main() {

    /* server socket config */

    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    Bind(sockfd, (SA*) &servaddr, sizeof(servaddr));

    /* init for random module, to simulate package loss */
    srand((unsigned int)time(NULL));

    /* create an operator to handle coming requests */
    Operator op = Operator(sockfd, (SA*) &cliaddr, sizeof(cliaddr));
    op.run();

    return 0;
}
