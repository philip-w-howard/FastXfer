#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "sendfile.h"

#define BUFFER_SIZE		1024*1024

void process_get(int fd, char *filename)
{
    int status;

    status = Send_Command(fd, "get");
    if (status != 0)
    {
        printf("Error in get command\n");
        return;
    }

    status = Send_Filename(fd, filename);
    if (status != 0)
    {
        printf("Error sending filename\n");
        return;
    }

    status = Recv_File(fd, filename);
    if (status != 0)
    {
        printf("Error receiving file\n");
        return;
    }
}

void process_put(int fd, char *filename)
{
    int filefd;
    int status;

    filefd = open(filename, O_RDONLY);
    if (filefd < 0)
    {
        printf("Unable to open '%s'\n", filename);
        return;
    }

    status = Send_Command(fd, "put");
    if (status != 0)
    {
        printf("Error in put command\n");
        return;
    }

    status = Send_Filename(fd, filename);
    if (status != 0)
    {
        printf("Error sending filename\n");
        return;
    }

    status = Send_File(fd, filefd);
    if (status != 0)
    {
        printf("Error sending file\n");
        return;
    }
}

void process_speed(int fd, char *size_str)
{
    int status;
    int size = atoi(size_str);
    char filename[256];
    char buffer[BUFFER_SIZE];

    status = Send_Command(fd, "put");
    if (status != 0)
    {
        printf("Error in put command\n");
        return;
    }

    strcpy(filename, "data.file");
    status = Send_Filename(fd, filename);
    if (status != 0)
    {
        printf("Error sending filename\n");
        return;
    }

    int filesize;
    int count;
    int blocks = 0;

    // get file size
    filesize = size*BUFFER_SIZE;

    sprintf(buffer, "%d\r\n", filesize);

    count = write(fd, buffer, strlen(buffer));
    if (count != strlen(buffer))
    {
        printf("Error sending file size: %d %s\n", count, buffer);
        return;
    }

    printf("Sending file of size %d\n", filesize);

    memset(buffer, 0, sizeof(buffer));
    /*
    int ii;
    for (ii=0; ii<BUFFER_SIZE; ii++)
    {
        buffer[ii] = (unsigned char)ii;
    }
    */

    count = BUFFER_SIZE;
    for (blocks=0; blocks < size; blocks++)
    {
        write(fd, buffer, count);
        filesize -= count;
        if (blocks % 100 == 0) printf("Sent %d bytes %d remaining\n", count, filesize);
    }

    if (filesize != 0)
    {
        printf("Error sending file: %d %d %d\n", size, blocks, filesize);
        return;
    }

    printf("Done sending file\n");

    return;
}


int main(int argc, char *argv[])
{
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[1000];
    char *token;

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        Fatal_Error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        Fatal_Error("ERROR connecting");

    while (1)
    {
        printf("myftp> ");
        fgets(buffer,sizeof(buffer),stdin);

        token = strtok(buffer, " \n");
        if (token == NULL || strlen(token)==0)
        {
            // do nothing
        }
        else if (strcmp(token, "quit") == 0)
        {
            break;
        }
        else if (strcmp(token, "put") == 0)
        {
            token = strtok(NULL, " \n");
            process_put(sockfd, token);
        }
        else if (strcmp(token, "get") == 0)
        {
            token = strtok(NULL, " \n");
            process_get(sockfd, token);
        }
        else if (strcmp(token, "speed") == 0)
        {
            token = strtok(NULL, " \n");
            process_speed(sockfd, token);
        }
        else if (strcmp(token, "ls") == 0)
        {
            printf("ls command is not implemented\n");
        }
        else
        {
            printf("command '%s' not recognized\n", token);
        }

    }
    close(sockfd);
    return 0;
}
