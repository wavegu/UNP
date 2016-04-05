#include <iostream>
#include "query.h"


using namespace std;


void udp_session(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen){
    char sendline[MAXLINE];
    Connect(sockfd, (SA*) pservaddr, servlen);

    /* Given a sendline,
     * construct a urlencode query,
     * and output the response */

    while (Fgets(sendline, MAXLINE, fp) != NULL){
        Query query = Query(sockfd, string(sendline));
        cout << get_timestamp() << " | " << query.get_answer() << endl;
        bzero(sendline, sizeof(sendline));
    }
}

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;

    /* Check parameters */
    if (argc != 2)
        err_quit("Usage: ./main <IPaddress>");

    /* UDP setting */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    /* start a udp session on sockfd */
    udp_session(stdin, sockfd, (SA*) &servaddr, sizeof(servaddr));

    return 0;
}
