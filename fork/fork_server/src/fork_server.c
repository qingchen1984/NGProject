// fork_server.c
// 
// Year 4 Networked Games Assignment 2015
//
// Team:	David Morton
//			Kevin Byrne
//			Derek O Brien
// 			add names here...
// 
//
// Description: The server in the Hangman game will create a socket, bind to a port, and will then 
// listen for incoming connections from clients. A signal handler is created to process signals from 
// child processes of the server in order to kill and release resources used by them on the system. 
// The SignalHandler() function implementation can be found in the socket.c file in the libsocket library.
// For each new TCP connection from a client the server will create a new process using the fork() function. 
// The listening socket inside the new process is then closed and control is passed to the play_hangman() function
// described in the game.c file. Once the game has ended the PlayHangmanServerTCP() function returns and the process
// will exit with code 0 meaning 'Success'
//
#include "../../../libhangman/hangman.h"

int main(int argc, char* argv[]) {

	int iListenSocketFileDescriptor;
	struct Address sAddress;
	pid_t childProcessID;
	int connfd;

	printf("Hangman server spinning up..\n");

	// Old version using gethostbyname()
	//iListenSocketFileDescriptor = Socket(AF_INET, SOCK_STREAM, 0);
	//Address(AF_INET, (struct Address*) &sAddress, strServerIPAddress, HANGMAN_TCP_PORT);
	//Bind(iListenSocketFileDescriptor, (struct sockaddr *) &sAddress.m_sAddress, sizeof(sAddress.m_sAddress));

	// Create a connection for the server
	iListenSocketFileDescriptor = InitConnection(NULL, "1071", TYPE_SERVER, SOCK_STREAM);

	// Listen for incoming TCP connections and set max limit of
	// listen queue
	ListenForConnections(iListenSocketFileDescriptor, MAX_LISTEN_QUEUE_SIZE);

	// Fork Implementations for server

	// Signal handler for terminated processes
	// Only needed when forking processes
	CreateSignalHandler();

	printf("Listening for incoming game connections\n");
	for( ; ; ) {

		// Accept all incoming TCP connections and return a file descriptor
		// used to communicate with the client.
		connfd = AcceptGameConnection(iListenSocketFileDescriptor, &sAddress);

		// There was no error in AcceptGameConnection()! Woo! Create a child process
		// to handle game for each client
		if( (childProcessID = fork()) == 0)
		{
			// CHILD
			//printf("child %d created\n", childProcessID);

			// Close the parents listen file descriptor in the child
			close(iListenSocketFileDescriptor);

			printf("Server starting a new game of Hangman\n");

			/* ---------------- Play_hangman () ---------------------*/
			PlayHangmanServerTCP(connfd, connfd);

			printf("Game Over disconnecting...\n");

			/*
			 *  On return exit to kill the process. The kernel will then
			 *  send a signal to the parent which is caught by the parents
			 *  SignalHandler() set in Signal()
			 */
			exit(0);
		}

		close(connfd);
	}
	return 0;
}



