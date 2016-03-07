extern "C"{
    #include "unp.h"
}

void reply(int sockfd, SA *pcliaddr, socklen_t clilen){
    socklen_t len;
    char message[MAXLINE];

    while (TRUE){
        len = clilen;
        int ret = Recvfrom(sockfd, message, MAXLINE, 0, pcliaddr, &len);
        Sendto(sockfd, message, ret, 0, pcliaddr, len);
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