extern "C"{
#include "unp.h"
}

void request(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen){
    char sendline[MAXLINE], recvline[MAXLINE+1];

    while (Fgets(sendline, MAXLINE, fp) != NULL){
        Sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
        int len = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
        recvline[len] = 0;
        Fputs(recvline, stdout);
    }
}

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc != 2)
        err_quit("Usage: ./main <IPaddress>");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
    request(stdin, sockfd, (SA*) &servaddr, sizeof(servaddr));

}