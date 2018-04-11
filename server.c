//
// Created by Rory Powell on 6/4/18.
//

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define PORT 1234

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
        perror("Error binding socket\n");
        return -1;
    }

    printf("Successfully bound to port %u\n", PORT);

    // listen to for connection on port
    listen(sockfd, 5);

    // accept the incoming connection request
    int clientlen = sizeof(client_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t *)&clientlen);
    printf("accepted new client\n");

    // do something with connection
    test_func(newsockfd);

    close(newsockfd);
    close(sockfd);

}

void test_func(int sock){

    const int NOT_AUTH = 0;

    char sendbuf[10000];
    char recbuf[1000];
    char *token;
    char *method;
    char *path;
    char *httpver;
    const char methoddelim[2] = " ";
    const char pathdelim[2] = " ";


    strcpy(sendbuf, "HTTP/1.0 200 OK\r\n\r\n");
    send(sock, sendbuf, strlen(sendbuf), 0);

    int state = NOT_AUTH;

//    while(1) {
        // wait for client response
        recv(sock, recbuf, 1000, 0);
//        printf("%s\n", recbuf);
        method = strtok(recbuf, methoddelim);
//        printf("method: %s\n", method);

        if (!strcmp(method, "GET")){
            // get request
            printf("Method: %s\n", method);
            path = strtok(NULL, pathdelim);
            printf("Path: %s\n", path);

        } else if (!strcmp(method, "POST")){
            // if a post request is sent
            printf("This server currently only implements HTTP/1.0\n");
        } else {
            printf("Unrecognised HTTP request\n");
        }

//        while(1){
//            recv(sock, recbuf, 1000, 0);
//            if (!strcmp(method, "GET")){
//                test_func(sock);
//            }
//        }

//    while( token != NULL ) {
//        printf( " %s\n", token);
//        token = strtok(NULL, s);
//    }

//        printf("%s\n", buf);
//
//        if (strncmp("GET", buf, 5)){
//            printf("get request\n");
//            strcpy(buf, "");
//            send(sock, buf, strlen(buf), 0);
//        }
//    }
}

