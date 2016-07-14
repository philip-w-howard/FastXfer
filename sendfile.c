/*
 * sendfile.c
 *
 *  Created on: Jul 13, 2016
 *      Author: philhow
 */

#include <errno.h>
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

#define BUFFER_SIZE (1024*1024)

void Fatal_Error(const char *msg)
{
    perror(msg);
    exit(1);
}

int Send_Error(int fd, char *error)
{
    int size = 0;

    write(fd, &size, sizeof(int));
    write(fd, error, strlen(error)+1);

    return 0;
}

int Recv_Filename(int fd, char *filename)
{
    int size;
    int count;

    printf("Getting filename size\n");
    count = read(fd, &size, sizeof(size));
    if (count != sizeof(size))
    {
        printf("Error reading filename len\n");
        return 1;
    }
    printf("Getting filename of length %x %x\n", size, ntohl(size));
    size = ntohl(size);

    count = read(fd, filename, size);
    if (count != size)
    {
        printf("Error reading filename\n");
        return 2;
    }

    printf("file: '%s'\n", filename);
    return 0;
}

int Send_Filename(int fd, char *filename)
{
    int count;
    unsigned char size;

    size = (unsigned char)strlen(filename);

    printf("Sending size: %d\n", size);
     count = write(fd, &size, sizeof(size));
    if (count != sizeof(size))
    {
        printf("Error sending size: %d\n", count);
        return 1;
    }

    printf("Sending filename: %s\n", filename);
    count = write(fd, filename, size);
    if (count != size)
    {
        printf("Error sending filename: %d\n", count);
        return 2;
    }

    printf("Done sending filename\n");
    return 0;
}

int Send_Command(int fd, char *command)
{
    int count;

    count = write(fd,command,strlen(command));
    if (count < 0)
    {
        printf("Error sending command\n");
        return 1;
    }

    count = write(fd,"\r\n", 2);
    if (count < 0)
    {
        printf("Error sending command\n");
        return 1;
    }

    return 0;
}

int Recv_Command(int fd, char *command)
{
    int count;

    count = read(fd,command,4);
    if (count != 4)
    {
        printf("Error reading command\n");
        return 1;
    }

    return 0;
}

int Send_File(int fd, int filefd)
{
    int filesize;
    int count;
    int blocks = 0;
    char buffer[BUFFER_SIZE];

    memset(buffer,0, sizeof(buffer));

    // get file size
    filesize = lseek(filefd, 0, SEEK_END);

    // rewind back to beginning
    lseek(filefd, 0, SEEK_SET);

    sprintf(buffer, "%d\r\n", filesize);

    count = write(fd, buffer, strlen(buffer));
    if (count != strlen(buffer))
    {
        printf("Error sending file size: %d %s\n", count, buffer);
        return 1;
    }

     printf("Sending file of size %d\n", filesize);

    while ((count = read(filefd, buffer, sizeof(buffer))) > 0)
    {
        write(fd, buffer, count);
        filesize -= count;
        if (++blocks % 50 == 0) printf("Sent %d bytes %d remaining\n", count, filesize);
    }

    if (filesize != 0)
    {
        printf("Error sending file: %d\n", filesize);
        return 2;
    }

    printf("Done sending file\n");

    return 0;
}

int Recv_File(int fd, char *filename)
{
    int filefd;
    int size;
    int count;
    char buffer[1000];

    filefd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
    if (filefd == -1)
    {
        printf("Error opening file: %s\n", filename);
        return 2;
    }

    count = read(fd, &size, sizeof(size));
    if (count != sizeof(size))
    {
        printf("Error reading file size: %d\n", count);
        return 3;
    }

    size = ntohl(size);
    if (size == 0)
    {
        read(fd, buffer, sizeof(buffer));
        printf("Error: %s\n", buffer);
        return 4;
    }

    printf("Waiting for file of size: %d\n", size);

    while (size > 0)
    {
        count = read(fd, buffer, sizeof(buffer));
        if (count <= 0)
        {
            printf("Error reading from network: %d %d\n", count, errno);
            close(filefd);
            return 4;
        }

        size -= count;
        count = write(filefd, buffer, count);
        if (count <= 0)
        {
            printf("Error writing file: %d %d\n", count, errno);
            close(filefd);
            return 5;
        }
    }

    close(filefd);
    printf("File written successfully\n");

    return 0;
}



