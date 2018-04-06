//
// Created by Rory Powell on 6/4/18.
//

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 8080

void test_func(int sock);

int main()
{

    struct sockaddr_in serv_addr, client_addr;

    int sockfd, newsockfd;

    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        printf("Can not open socket, port in use.\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = 0;
    serv_addr.sin_addr.s_addr = INADDR_ANY;


    // Bind to the socket
    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in) ) < 0)
    {
        printf("Error binding socket\n");
        return -1;
    }

    printf("Successfully bound to port %u\n", PORT);

    // listen to for connection on port
    listen(sockfd, 5);

    // accept the incoming connection request
    int clientlen = sizeof(client_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t *)&clientlen);

    // do something with connection
    test_func(newsockfd);

}

void test_func(int sock){

    char buf[1000];
    strcpy(buf, "connection test success!\n");

    send(sock, buf, strlen(buf), 0);
}