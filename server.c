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
#include <server.h>


int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr;
    int sockfd, port;
    pthread_t thread;

    if(argc < 3)
    {
        perror("Usage: ./server [port number] [root directory]\n");
        exit(EXIT_FAILURE);
    }

    if ((atoi(argv[1]) > 0))
    {
        port = atoi(argv[1]);
    }
    else
        {
        perror("Please specify a valid port number\n");
        exit(EXIT_FAILURE);
        }

    strcpy(ROOT, argv[2]);

    // create socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Can not open socket, port in use.\n");
        exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
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
            exit(EXIT_FAILURE);
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
                        exit(EXIT_FAILURE);
                    }
                pthread_detach(thread);
                sched_yield();
            }
    }
}

int fileValid(char path[])
{
    //printf("path: \"%s\"\n", path);
    return open(path, O_RDONLY, 0);
}

char * get_filetype(char *path)
{
    int i = 0, j = 0;
    int eofFilename = 0;
    char *extension;

    extension = (char*)malloc(sizeof(path)+1);

    for (i = 0; i <= strlen(path); i++)
    {
        if(path[i] == '.'){
            eofFilename = 1;
        }
        if (eofFilename){

            extension[j] = path[i+1];
            j++;
        }
    }
//    printf("%s\n", extension);
    return extension;
}

int get_filesize(char *path)
{
    FILE *file;
    int filesize = 0;

    file = fopen(path, "r");

    fseek(file, 0L, SEEK_END);
    filesize = ftell(file);
    fclose(file);
    return filesize;

}

void get_req_reply(int sock, char path[])
{
    char buf[1024];
    char *filetype, *filesizestr;

    filetype = (char*)malloc(sizeof(buf));
    filesizestr = (char*)malloc(sizeof(buf));
    filetype = get_filetype(path);

    strcpy(buf, VALIDREQHEAD);
    sprintf(filesizestr, "Content-Length: %d\r\n", get_filesize(path));
    strcat(buf, filesizestr);

    if(!strcasecmp(filetype, HTML))
    {
        strcat(buf, "Content-Type: text/html;\r\n\n");
    }
    else if (!strcasecmp(filetype, JAVASCRIPT))
    {
        strcat(buf, "Content-Type: text/js;\r\n\n");
    }
    else if (!strcasecmp(filetype, CSS))
    {
        strcat(buf, "Content-Type: text/css;\r\n\n");
    }
    else if (!strcasecmp(filetype, JPEG))
    {
        strcat(buf, "Content-Type: image/jpeg;\r\n\n");
    }
    else
    {
        strcat(buf, "Content-Type: unknown;\r\n\n");
    }

//    strcat(buf, NEWLINE);

    write(sock, buf, strlen(buf));

}

void *parse_HTTP(void *sock)
{

    char buf[1000], path[1000], newpath[1000];
    char *ptr;
    int bytes_read = 0, i = 0;

    int socket = *((int *) sock);
    free(sock);

    bytes_read = recv(socket, buf, 1000, 0);
    buf[bytes_read-1] = EOF_BUF;

    // wait for client response
    if(bytes_read == -1)
    {
        perror("client disconnected\n");
        return 0;
    }


    strcpy(path, ROOT);
    strcpy(newpath, ROOT);
    strcat(newpath, "/");

    if(strncmp("GET ", buf, 4) == 0)
    {
        ptr = buf + 4;
        if(*ptr == '/')
        {
            int pathvalid = 0;
            i = strlen(path);
            while(*ptr != ' ')
            {
                path[i] = *ptr;
                i++;
                ptr++;
                pathvalid++;
            }
            path[i] = '\0';

            if(!strcmp(newpath, path))
            {
                // if the path specified is '/' serve index.html
                strcat(path, "index.html");
            }
//            printf("newpath: %s\n", newpath);
//            printf("path: %s\n", path);
        }
        else
        {
            strcpy(buf, INVALIDREQHEAD);
            write(socket, buf, strlen(buf));
            perror("404 file not found\n");
            pthread_exit(NULL);
        }

//        printf("path: %s \n\n", path);

        if (get_filetype(path) == NULL) {
            perror("Incomplete request\n");
            pthread_exit(NULL);
        }

        // get request
        if (fileValid(path) != -1) {
            get_req_reply(socket, path);
            pthread_exit(NULL);
        } else {
            strcpy(buf, INVALIDREQHEAD);
            write(socket, buf, strlen(buf));
            perror("404 file not found\n");
            pthread_exit(NULL);
        }
    }
    else
    {
//        printf("%s\n",buf);
        perror("Unrecognised HTTP request\n");
        pthread_exit(NULL);
    }
    close(socket);
    pthread_exit(NULL);
    return 0;

}

