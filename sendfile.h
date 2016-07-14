/*
 * sendfile.h
 *
 *  Created on: Jul 13, 2016
 *      Author: philhow
 */

#ifndef SENDFILE_H_
#define SENDFILE_H_

void Fatal_Error(const char *msg);

int Send_Error(int fd, char *error);

int Send_Command(int fd, char *command);
int Recv_Command(int fd, char *command);

int Send_Filename(int fd, char *filename);
int Recv_Filename(int fd, char *filename);

int Send_File(int fd, int filefd);
int Recv_File(int fd, char *filename);

#endif /* SENDFILE_H_ */
