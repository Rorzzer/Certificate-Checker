//
// Created by Rory Powell on 6/4/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <pthread.h>


//#define PORT 1234
#define BYTES 1024
#define DIR "dir"

void *parse_HTTP(void *sock);
//int create_socket(int domain, int type, int proto);

int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr;
    int sockfd, /*newsockfd,*/ port;
    char path[100];
    pthread_t thread;

    if ((atoi(argv[1]) > 0))
    {
        port = atoi(argv[1]);
    } else
        {
        printf("Please specify a valid port number\n");
        exit(1);
        }

    strcpy(path, argv[2]);

    // create socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Can not open socket, port in use.\n");
        exit(1);
    }
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        serv_addr.sin_addr.s_addr = 0;
        serv_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind to the socket to port
    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in) ) < 0)
    {
        perror("Error binding socket\n");
        exit(1);
    }
    printf("Successfully bound to port %u\n", port);

    // listen to for connection on port
    listen(sockfd, 5);

    while(1)
    {
        struct sockaddr_in client_addr = {0};
        int newsock = 0;
        int clientlen = sizeof(client_addr);

        newsock = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t *)&clientlen);

        if(newsock  == -1)
        {
            perror("Cannot accept new connection\n");
            exit(1);
        } else
            // thread for each connection
            {
                // create a void ptr argument to send each thread
                int *arg = malloc(sizeof(*arg));
                *arg = newsock;

                if((pthread_create(&thread, NULL, parse_HTTP, arg)) != 0)
                    {
                        perror("Could not create thread\n");
                        exit(1);
                    }

//              printf("thread\n");
                pthread_detach(thread);
                sched_yield();
            }
    }

    close(sockfd);
}


void *parse_HTTP(void *sock)
{

    const int NOT_AUTH = 0;

    char sendbuf[10000], recbuf[1000], to_send[BYTES], newpath[1000];
    char *token, *method, *path, *httpver;
    const char methoddelim[2] = " ";
    const char pathdelim[2] = " ";
    int fd, bytes_read;

    int socket = *((int *) sock);
    free(sock);
//  int socket = (int)sock;

    strcpy(sendbuf, "HTTP/1.0 200 OK\r\n\r\n");
    send(socket, sendbuf, strlen(sendbuf), 0);

    int state = NOT_AUTH;

//    while(1) {
    // wait for client response
    if((recv(socket, recbuf, 1000, 0)) == -1)
    {
        perror("client disconnected\n");
        return 0;
    }

//        printf("%s\n", recbuf);
    method = strtok(recbuf, methoddelim);

        printf("method: %s\n", method);



    if ((method != NULL) && !strcmp(method, "GET"))
    {
        // get request
        printf("Method: %s\n", method);
        path = strtok(NULL, pathdelim);
        printf("Path: %s\n", path);

        if (strcmp(path, "/"))
        {

            strcpy(sendbuf, "HTTP/1.0 200 OK"
                            "Server: Rorys Server v1.0\n"
                            "Content-Type: image/jpeg\r\n\r\n");
            send(socket, sendbuf, strlen(sendbuf), 0);

            strcpy(newpath, DIR);
            strcpy(&newpath[strlen(DIR)], path);

        } else
            {
            path = strcat(path, "index.html");

            }

    } else if ((method != NULL) && !strcmp(method, "POST"))
        {
            // if a post request is sent
            printf("This server currently only implements HTTP/1.0\n");

        } else
            {
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

    return 0;
}

