//
// Created by Rory Powell on 16/4/18.
//

#ifndef COMP30023_2018_PROJECT_1_SERVER_H
#define COMP30023_2018_PROJECT_1_SERVER_H
#endif //COMP30023_2018_PROJECT_1_SERVER_H

#define MAXBUF 1024
#define MAXEXT 50
#define MAXPATH 1000
#define DIR "dir"
#define SERVER "RPOWELL HTTP Server v1.0\r\n"
#define VERSION "HTTP/1.0"
#define EOF_BUF '\0'
#define VALIDREQHEAD "HTTP/1.0 200 OK\r\n"
#define INVALIDREQHEAD "HTTP/1.0 404\r\n\n"
#define CONTENTLENGTH "Content-Length: "
#define NEWLINE "\r\n\n"


// filetypes
#define HTML "html"
#define JAVASCRIPT "js"
#define CSS "css"
#define JPEG "jpg"


char ROOT[500];

void *parse_HTTP(void *sock);
int fileValid(char path[]);
void get_req_reply(int sock, char path[], char extension[]);
int get_filetype(char *path, char extension[]);
int get_filesize(char *path);


