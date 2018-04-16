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
#define SERVER "RPOWELL HTTP Server v1.0\r\n"
#define VERSION "HTTP/1.0"
#define EOF_BUF '\0'
#define VALIDREQHEAD "HTTP/1.0 200 OK\r\n"

// filetypes
#define HTML "html"
#define JAVASCRIPT "js"
#define CSS "css"
#define JPEG "jpg"



void *parse_HTTP(void *sock);
int fileValid(char path[]);
void get_req_reply(int sock, char path[]);
char * get_filetype(char *path);
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
    }
    else
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

    // set socket details
    memset(&serv_addr, 0, sizeof(serv_addr)); // zero the struct before adding values below
    serv_addr.sin_family = AF_INET; //IPv4 address
    serv_addr.sin_port = htons(port); //Port number
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
        memset(&client_addr, 0, sizeof(client_addr)); // zero the struct
        int newsock = 0;
        int clientlen = sizeof(client_addr);

        newsock = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t *)&clientlen);

        if(newsock  == -1)
        {
            perror("Cannot accept new connection\n");
            exit(1);
        }
        else
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

int fileValid(char path[])
{
//    printf("Path: \"%s\" \n", path);
    //printf("%d \n", open(path, O_RDONLY, 0));
    return open(path, O_RDONLY, 0);
}

char * get_filetype(char *path)
{
    int i = 0, j = 0;
    int eofFilename = 0;
    char *extension;

    extension = (char*)malloc(sizeof(path)+1);


    for (i = 0; i <= sizeof(path); i++)
    {
        if(path[i] == '.'){
            eofFilename = 1;
        }
        if (eofFilename){

            extension[j] = path[i+1];
            j++;
        }
    }
//    extension[sizeof(extension)+2] =
    return extension;
}

void get_req_reply(int sock, char path[])
{
    char buf[1024];
    char *filetype;
    int filesize;


    filetype = (char*)malloc(sizeof(buf)+1);
    filetype = get_filetype(path);

    strcpy(buf, VALIDREQHEAD);


    if(!strcmp(filetype, HTML))
    {
        strcat(buf, "Content-Type: text/html;\r\n\n");
    }
    else if (!strcmp(filetype, JAVASCRIPT))
    {
        strcat(buf, "Content-Type: text/js;\r\n\n");
    }
    else if (!strcmp(filetype, CSS))
    {
        strcat(buf, "Content-Type: text/css;\r\n\n");
    }
    else if (!strcmp(filetype, JPEG))
    {
        strcat(buf, "Content-Type: image/jpeg;\r\n\n");
    }
    else
    {
        strcat(buf, "Content-Type: unknown;\r\n\n");
    }

    //printf("filetype: %s\n", get_filetype(path));

//    printf("file type: %s\n", filetype);



    //strcat(buf, filesize); // get the file size
     // get the file type
    //printf("buffer: %s\n", buf);
    write(sock, buf, strlen(buf));

}

void *parse_HTTP(void *sock)
{

    const int NOT_AUTH = 0;

    char sendbuf[10000], buf[1000], to_send[BYTES], newpath[1000];
    char *token, *method, *path, *httpver, *filetype;
    const char delim[1] = " ";
    const char pathdelim[2] = " ";
    int fd, bytes_read, buf_len;

    int socket = *((int *) sock);
    free(sock);
//  int socket = (int)sock;


//    strcpy(sendbuf, "HTTP/1.0 200 OK\r\n\r\n");
//    send(socket, sendbuf, strlen(sendbuf), 0);

    //int state = NOT_AUTH;

//    while(1) {

    bytes_read = recv(socket, buf, 1000, 0);
    buf[bytes_read] = EOF_BUF;

    // wait for client response
    if(bytes_read == -1)
    {
        perror("client disconnected\n");
        return 0;
    }

//      printf("%s\n", recbuf);
    method = (char*)malloc(sizeof(buf)+1);
    method = strtok(buf, delim);
    //printf("method: \"%s\"\n", method);

    path = (char*)malloc(sizeof(buf)+1);
    path = strtok(NULL, delim);
    path[sizeof(path)+2] = EOF_BUF;
    //printf("Path: %s\n", path);

    //printf("filetype: %d\n", fileValid("dir\\script.js"));

    if(method == NULL || path == NULL || get_filetype(path) == NULL)
    {
        perror("Incomplete request\n");
        pthread_exit(NULL);
    }

    if (!strcmp(method, "GET"))
    {
        // get request
      if(fileValid(path) != -1)
      {
          get_req_reply(socket, path);
//          filetype = get_filetype(path);
          //printf("filetype: %s\n", filetype);

      }
      else
          {
           perror("404 file not found\n");
          }

//        strcpy(sendbuf, "HTTP/1.0 200 OK\r\n\r\n");
//        send(socket, sendbuf, strlen(sendbuf), 0);

//        if (strcmp(path, "/"))
//        {
//
//            strcpy(sendbuf, "HTTP/1.0 200 OK\n"
//                            "Server: Rorys Server v1.0\n"
//                            "Content-Type: image/jpeg\r\n\r\n");
//            send(socket, sendbuf, strlen(sendbuf), 0);
//
//            strcpy(newpath, DIR);
//            strcpy(&newpath[strlen(DIR)], path);
//
//        } else
//            {
//            path = strcat(path, "index.html");
//
//            }

    }
    else if (!strcmp(method, "POST"))
        {
        // if a post request is sent
        printf("This server currently only implements HTTP/1.0\n");
        pthread_exit(NULL);
        }
        else
            {
            printf("Unrecognised HTTP request\n");
            pthread_exit(NULL);
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

