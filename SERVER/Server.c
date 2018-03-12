/*************************************************************************
 > File Name: Server.c
 > Author: David Yao  
 > Email: david.yao.sh.dy@gmail.com
 > Created Time: Wed 28 Feb 2018 12:46:47 CST
************************************************************************/

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ServerConfig.h"
#ifdef USE_WRAP
#include "Wrap.h"
#endif

#define SERVER_PORT 8080
#define LENGTH 20
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512

static int setup_socket(unsigned int port, unsigned int listenlen){
    int fd = -1;
    int opt = 1;
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(port);

    fd = Socket(PF_INET, SOCK_STREAM, 0);

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    Bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    Listen(fd, listenlen);
    
    return fd;
}

static ssize_t request_received(int fd, char* buffer){
    return Read(fd, buffer, BUFFER_SIZE); 
}

static void file_transmit(int fd, char* buffer, char* file_name){
    FILE *fp = fopen(file_name, "r");
    int length = 0;

    if (NULL == fp)
    {
        fprintf(stdout, "File:%s Not Found\n", file_name);
    }

    else
    {
        while ((length = fread(buffer, sizeof(char), BUFFER_SIZE,fp)) > 0)
        {
            if (Write(fd, buffer,BUFFER_SIZE) < 0)
            {
                fprintf(stdout, "Send File:%s Failed./n", file_name);
                break;
            }
            bzero(buffer, BUFFER_SIZE);
        }
           
        fclose(fp);
        fprintf(stdout, "File:%s Transfer Successful!\n", file_name);
    }   
} 

int main(void)
{
    struct sockaddr_in client_addr;
    socklen_t client_len;
    int listen_fd = -1;
    int conn_fd = -1;

    char buffer[BUFFER_SIZE];
    char file_name[FILE_NAME_MAX_SIZE + 1];

    fprintf(stdout,"Server Version %d.%d\n",
            SERVER_VERSION_MAJOR,
            SERVER_VERSION_MINOR);


    if(-1 == (listen_fd = setup_socket(SERVER_PORT, LENGTH)))
    {
        perror("Server Socket Setup Failed:");
        exit(1);
    }
    while(1)
    {
        client_len = sizeof(client_addr);

        if(-1 == (conn_fd = Accept(listen_fd, (struct sockaddr*)&client_addr, &client_len)))
        {
            perror("Server Accept Failed:");
            break;
        }
        
        bzero(buffer, BUFFER_SIZE);
        bzero(file_name, FILE_NAME_MAX_SIZE + 1);

        if (request_received(conn_fd, buffer) < 0)
        {
            perror("Server Receive Data Failed:");
            break;
        }

        strncpy(file_name, buffer, strlen(buffer) > FILE_NAME_MAX_SIZE? FILE_NAME_MAX_SIZE:strlen(buffer));
        bzero(buffer, BUFFER_SIZE);

        file_transmit(conn_fd, buffer, file_name);   

        Close(conn_fd);
    }

    Close(listen_fd);
    return 0;
}

