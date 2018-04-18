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
#include <sys/uio.h>
#include <sys/types.h>
#include <assert.h>



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
                int *arg = malloc(sizeof(*arg)+1);
                assert(*arg);
                *arg = newsock;

                if((pthread_create(&thread, NULL, parse_HTTP, arg)) != 0)
                    {
                        perror("Could not create thread\n");
                        exit(EXIT_FAILURE);
                    }

                pthread_detach(thread);
                sched_yield();
                free(arg);
            }
    }
}

int fileValid(char path[])
{
    return open(path, O_RDONLY, 0);
}

int get_filetype(char *path, char extension[])
{
    // gets the file extension
    char *ext = strrchr(path, '.');

    if (ext == NULL)
    {
    return 0;
    }
    strcpy(extension, ext + 1);
    return 1;
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

void get_req_reply(int sock, char path[], char extension[])
{

    char buf[1024], ext[100];
    char *filesizestr;

    strcpy(ext, extension);

    filesizestr = (char*)malloc(sizeof(buf)+1);
    assert(filesizestr);


    strcpy(buf, VALIDREQHEAD);
    sprintf(filesizestr, "Content-Length: %d\r\n", get_filesize(path));
    strcat(buf, filesizestr);

    if(!strcmp(ext, HTML))
    {
        strcat(buf, "Content-Type: text/html\r\n\n");
    }
    else if (!strcmp(ext, JAVASCRIPT))
    {
        strcat(buf, "Content-Type: text/javascript\r\n\n");
    }
    else if (!strcmp(ext, CSS))
    {
        strcat(buf, "Content-Type: text/css\r\n\n");
    }
    else if (!strcmp(ext, JPEG))
    {
        strcat(buf, "Content-Type: image/jpeg\r\n\n");
    }
    else
    {
        strcat(buf, "Content-Type: unknown\r\n\n");
    }


    if((write(sock, buf, strlen(buf)) == -1))
    {
        perror("Write failed.\n");
    }

}

void *parse_HTTP(void *sock)
{

    char buf[1000], path[1000], newpath[1000], extension[1000];
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
            path[i+1] = '\0';

            if(!strcmp(newpath, path))
            {
                // if the path specified is '/' serve index.html
                strcat(path, "index.html");
            }
        }
        else
        {
            // not a valid path
            strcpy(buf, INVALIDREQHEAD);

            if((write(socket, buf, strlen(buf)) == -1))
            {
                perror("Write failed.\n");
            }
            perror("Not a valid path\n");
            pthread_exit(NULL);
        }

        // get request
        if (fileValid(path) != -1) {

            if ((get_filetype(path, extension)) < 1) {
                perror("Incomplete request\n");
                pthread_exit(NULL);
            }

            get_req_reply(socket, path, extension);

            FILE *file;

            file = fopen(path, "r");

            char filebuf[get_filesize(path) + 1];

            if((fread(filebuf, get_filesize(path), 1, file) == -1))
            {
                perror("Read failed.\n");
            }

            send(socket, filebuf, get_filesize(path), 0);
            fclose(file);

            pthread_exit(NULL);
        } else {
            // file not found
            strcpy(buf, INVALIDREQHEAD);

            if((write(socket, buf, strlen(buf)) == -1))
            {
                perror("Write failed.\n");
            }
            perror("404 file not found\n");
            pthread_exit(NULL);
        }
    }
    else
    {
        perror("Unrecognised HTTP request\n");
        pthread_exit(NULL);
    }
    close(socket);
    pthread_exit(NULL);
    return 0;

}

