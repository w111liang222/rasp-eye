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

#define SERVER_PORT 8080
#define LENGTH 20
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512

int main(void)
{
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    int server_socket_fd = socket(PF_INET,SOCK_STREAM, 0);
    if (server_socket_fd < 0)
    {
        perror("Create Socket Failed:");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (-1 == (bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))))
    {
        perror("Server Bind Failed:");
        exit(1);
    }

    if (-1 == (listen(server_socket_fd, LENGTH)))
    {
        perror("Server Listen Failed:");
        exit(1);
    }

    while(1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_length = sizeof(client_addr);

        int new_server_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, &client_addr_length);

        if (new_server_socket_fd < 0)
        {
            perror("Server Accept Failed:");
            break;
        }
        
        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);
        if (recv(new_server_socket_fd, buffer, BUFFER_SIZE,0) < 0)
        {
            perror("Server Receive Data Failed:");
            break;
        }
        printf("Server Receive Client Request for File: %s\n", buffer);

        char file_name[FILE_NAME_MAX_SIZE + 1];
        bzero(file_name, FILE_NAME_MAX_SIZE + 1);
        strncpy(file_name, buffer, strlen(buffer) > FILE_NAME_MAX_SIZE? FILE_NAME_MAX_SIZE:strlen(buffer));

        printf("Requesting File: %s\n", file_name);

        FILE *fp = fopen(file_name, "r");
        if (NULL == fp)
        {
            printf("File:%s Not Found\n", file_name);
        }

        else
        {
            bzero(buffer, BUFFER_SIZE);
            int length = 0;
            while ((length = fread(buffer, sizeof(char), BUFFER_SIZE,fp)) > 0)
            {
                if (send(new_server_socket_fd, buffer, length, 0) < 0)
                {
                    printf("Send File:%s Failed./n", file_name);
                    break;
                }
                bzero(buffer, BUFFER_SIZE);
            }
            
            fclose(fp);
            printf("File:%s Transfer Successful!\n", file_name);
        }
        
        close(new_server_socket_fd);
    }

    close(server_socket_fd);
    return 0;
}

