/* Network server for hangman game */
/* File: hangserver.c */

#include "../includes/Definitions.h"
#include "../../../libsocket/Sockets.h"
#include "../includes/Game.h"

int main(int argc, char* argv[]) {
	int iListenSocketFileDescriptor;
	char * strServerIPAddress;
	struct Address sAddress;
	struct Address sClientAddress;

	pid_t childProcessID;
	int connfd;
	socklen_t client_len;

	strServerIPAddress = "0.0.0.0";

	int iClientAddrLen;
	int iStatus;
	char* buffer[MAX_BUF_SIZE];

	printf("Server: initialising\n");

	iListenSocketFileDescriptor = Socket(AF_INET, SOCK_DGRAM, 0);

	// Use command line input to pass in the hostname and service port number.
	// AddressIPX("www.google.com", "1071", struct addrinfo *hints, struct addrinfo** result);
	Address(AF_INET, (struct Address*) &sAddress, strServerIPAddress, HANGMAN_TCP_PORT);

	Bind(iListenSocketFileDescriptor, (struct sockaddr *) &sAddress.m_sAddress, sizeof(sAddress.m_sAddress));

	//Listen(iListenSocketFileDescriptor, MAX_LISTEN_QUEUE_SIZE);

	// signal handler for terminated processes
	//Signal(SIGCHLD, signalHandler);

	iClientAddrLen = sizeof(sClientAddress);
	iStatus = recvfrom(iListenSocketFileDescriptor, buffer, MAX_BUF_SIZE, 0, (struct sockaddr*) &sClientAddress, &iClientAddrLen);

	printf("%s", buffer);

	strcat(buffer, "OK!\n");

	iStatus = sendto(iListenSocketFileDescriptor, buffer, strlen(buffer) + 1, 0, (struct sockaddr*) &sClientAddress, sizeof(sClientAddress));

	close(iListenSocketFileDescriptor);
	/*
	printf("listening for connections\n");
	for( ; ; ) {
		client_len = sizeof(sAddress.m_sAddress);
		// Accept connections from clients
		if ((connfd = accept(iListenSocketFileDescriptor, (struct sockaddr *) &sAddress.m_sAddress, &client_len)) < 0)
		{
			// There was an error (interrupt)
			if( errno == EINTR )
			{
				// Try another Accept() in the event of a system call interrupt
				continue;
			}
			else
			{
				// There was an error other than an interrupt so close the Parent process
				perror("Accept error");
				exit(3);
			}
		}

		// There was no error in Accept()! Woo! Create a child process
		if( (childProcessID = fork()) == 0)
		{
			// CHILD
			//printf("child %d created\n", childProcessID);

			// close the parents listen file descriptor in the child
			close(iListenSocketFileDescriptor);

			// ---------------- Play_hangman () ---------------------
			play_hangman(connfd, connfd);


			 // On return exit to kill the process. The kernel will then
			 // send a signal to the parent which is caught by the parents
			 // signalHandler() set in Signal()

			exit(0);
		}

		close(connfd);
	}
	*/
	return 0;
}



