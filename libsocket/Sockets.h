/*
 * Definitions.h
 *
 *  Created on: 4 Oct 2015
 *      Author: david
 */

#ifndef INCLUDES_SOCKETS_H_
#define INCLUDES_SOCKETS_H_

#include <stdio.h> // perror()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h> // exit(),
#include <unistd.h> // read(), write(), fork()

#define MAX_BUF_SIZE 4096
#define MAX_LISTEN_QUEUE_SIZE 1024

struct Address{
	struct sockaddr_in m_sAddress; /* Server's address assembled here */
	struct hostent * m_sHost_info;
};

int Socket(int family, int type, int protocol);
void Address(int family, struct Address* address, char* ipAddress, int portNumber);
void Connect(int socketFileDescriptor, const struct sockaddr* socketAddress, socklen_t socketSize);
int Select(int maxFileDescriptorsPlus1, fd_set *readFileDescriptorSet, fd_set *writeFileDescriptorSet, fd_set *exceptFileDescriptorSet, struct timeval *timeout);
ssize_t Read(int fileDescriptor, void *buffer, size_t numberOfBytes);
void Write(int fileDescriptor, void *buffer, size_t numberOfBytes);
void Shutdown(int fileDescriptor, int shutdownOption);
int Max(int x, int y);
void signalHandler(int signalNumber);
void Signal(int signalNumber, void* signalHandler);
void multiplexStdinFileDescriptor(FILE* fp, int socketFileDescriptor);

#endif /* INCLUDES_SOCKETS_H_ */