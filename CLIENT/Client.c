/*************************************************************************
 > File Name: Client.c
 > Author: David Yao  
 > Email: david.yao.sh.dy@gmail.com
 > Created Time: Wed 28 Feb 2018 14:08:38 CST
************************************************************************/
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ClientConfig.h"
#ifdef USE_WRAP
#include "Wrap.h"
#endif


#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512
static int connect_to(const char* ip, unsigned int port){
    int fd = -1;
    struct sockaddr_in client_addr, server_addr;
    bzero(&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);

    bzero(&client_addr, sizeof(client_addr));
    server_addr.sin_family = AF_INET;

    fd = Socket(AF_INET, SOCK_STREAM, 0);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) == 0)
    {
        perror ("Server IP Address Error:");
        exit(1);
    }
    server_addr.sin_port = htons(port);
    socklen_t server_addr_length = sizeof(server_addr);

    Connect(fd, (struct sockaddr*)&server_addr, server_addr_length);

    return fd;
}

static ssize_t send_request(int fd, char* file_name, char* buffer){
    printf("Please Input File Name on Server: \n");
    scanf("%s", file_name);

    strncpy(buffer, file_name, strlen(file_name) > BUFFER_SIZE? BUFFER_SIZE:strlen(file_name));

    return Write(fd, buffer, BUFFER_SIZE);
}

static void file_received(int fd, char* file_name, char* buffer){
    
    int length = 0;
    FILE *fp = fopen(file_name, "w");
    if (NULL == fp)
    {
        printf("File:\t%s Can Not Open To Write\n", file_name);
        exit(1);
    }

    while((length = Read(fd, buffer, BUFFER_SIZE)) > 0)
    {
        if (fwrite(buffer, sizeof(char), length, fp) < length)
        {
            printf("File:\t%s Write Failed\n", file_name);
            break;
        }
        bzero(buffer, BUFFER_SIZE);
    }

    printf("Receive File:\t%s From Server IP Successful!\n", file_name);
    fclose(fp);
}

int main(int argc, char **argv)
{
    int conn_fd = -1;
    char file_name[FILE_NAME_MAX_SIZE + 1];
    char buffer[BUFFER_SIZE];

    if (argc < 2)
    {
        perror("Please Enter an IP Address: ");
        exit(1);
    }

    if (-1 == (conn_fd = connect_to(argv[1], SERVER_PORT)))
    {
        perror ("Connect to Server Failed: ");
        exit(1);
    }

    bzero(file_name, FILE_NAME_MAX_SIZE + 1);
   
    bzero(buffer, BUFFER_SIZE);

    if (send_request(conn_fd, file_name, buffer) < 0)
    {
        perror("Send File Name Failed:");
        exit(1);
    }
    
    bzero(buffer, BUFFER_SIZE);

    file_received(conn_fd, file_name, buffer);

    Close(conn_fd);
    return 0;

}
