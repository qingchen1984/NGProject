
#include <syslog.h> // syslog()
#include <string.h> // strlen()
#include <unistd.h> // gethostname(), write()
#include <stdlib.h>
#include "hangman.h"

char *word[] = {
#include "words"
		};

extern time_t time();

void InitGameSessions()
{
	int i;
	for(i = 0; i < MAX_GAME_SESSIONS; i++)
	{
		strcpy(gameSessions[i].strUsername, "null");
		gameSessions[i].cGameState = 'U'; // unknown
		gameSessions[i].iLives = 0;
		gameSessions[i].iRandomWordLength = 0;
		gameSessions[i].iSequenceNumber = 0;
		gameSessions[i].strRandomWord = "null";
	}
}

void PrintActiveGameSessions()
{
	printf("active game sessions:\n");
	int i;
	for(i = 0; i < MAX_GAME_SESSIONS; i++)
	{
		printf("Index %d Username: %s\n", i, gameSessions[i].strUsername);
		if(strcmp(gameSessions[i].strUsername, "null") != 0)
		{
			//printf("Index %d Username: %s\n", i, gameSessions[i].strUsername);
		}
	}
}

void PrintGameSession(struct GameSession *gameSession)
{
	printf("Username: %s\n", gameSession->strUsername);
	printf("Lives: %d\n", gameSession->iLives);
	printf("Secret word: %s\n", gameSession->strRandomWord);
	printf("Progress: %s\n", gameSession->strPartWord);
}

struct GameSession *FindGameSession(char* username)
{
	printf("Searching for game session with %s...\n", username);
	int i;
	for(i = 0; i < MAX_GAME_SESSIONS; i++)
	{
		if(strcmp(gameSessions[i].strUsername, username) == 0)
		{
			printf("Game session found!\n");
			//return gameSessions[i].iSessionId; // return game session id
			return &gameSessions[i];
		}
	}

	// no game session found, create a new game session
	for(i = 0; i < MAX_GAME_SESSIONS; i++)
	{
		if(strcmp(gameSessions[i].strUsername, "null") == 0)
		{
			printf("No game session. Empty slot found! Creating new game...\n");
			strcpy(gameSessions[i].strUsername, username);
			gameSessions[i].iSessionId = i;
			//return gameSessions[i].iSessionId; // return game session id
			return &gameSessions[i];
		}
	}

	// there are no game session slots left
	return NULL;
}

void EndGameSession(struct GameSession *gameSession)
{
	strcpy(gameSession->strUsername, "null");
	gameSession->cGameState = 'U';
	gameSession->iSequenceNumber = 0;
}

// Play hangman using libsocket MultiplexIO()
void PlayHangmanClientTCP(FILE *filePointer, int socketFileDescriptor)
{
	// Use libsockets MultiplexIO to communicate over the network
	// with the server.
	MultiplexIO(filePointer, socketFileDescriptor);
}

void PlayHangmanServerTCP(int in, int out)
{

	char * whole_word, part_word[MAXLEN], guess[MAXLEN], outbuf[MAXLEN];

	int lives = MAX_LIVES;
	int game_state = 'I'; //I = Incomplete
	int i, good_guess, word_length;
	char hostname[MAXLEN];

	gethostname(hostname, MAXLEN);
	sprintf(outbuf, "Playing hangman on host %s: \n \n", hostname);
	Write(out, outbuf, strlen(outbuf));


	/* Pick a word at random from the list */
	srand((int) time((long *) 0)); /* randomize the seed */

	whole_word = word[rand() % NUM_OF_WORDS];
	word_length = strlen(whole_word);
	syslog(LOG_USER | LOG_INFO, "server chose hangman word %s", whole_word);

	/* No letters are guessed Initially */
	for (i = 0; i < word_length; i++)
		part_word[i] = '-';

	part_word[i] = '\0';

	sprintf(outbuf, "%s %d \n", part_word, lives);
	Write(out, outbuf, strlen(outbuf));

	while (game_state == 'I')
	/* Get a letter from player guess */
	{
		while (read(in, guess, MAXLEN) < 0) {
			if (errno != EINTR)
				exit(4);
			printf("re-read the startin \n");
		} /* Re-start read () if interrupted by signal */
		good_guess = 0;
		for (i = 0; i < word_length; i++) {
			if (guess[0] == whole_word[i]) {
				good_guess = 1;
				part_word[i] = whole_word[i];
			}
		}
		if (!good_guess)
			lives--;
		if (strcmp(whole_word, part_word) == 0)
			game_state = 'W'; /* W ==> User Won */
		else if (lives == 0) {
			game_state = 'L'; /* L ==> User Lost */
			strcpy(part_word, whole_word); /* User Show the word */
		}
		sprintf(outbuf, "%s %d \n", part_word, lives);
		Write(out, outbuf, strlen(outbuf));
	}
}

int PlayHangmanServerUDP(int clientFileDescriptor, struct Address client, struct GameSession* gameSession, char* message) {

	char *whole_word;
	char outbuf[MAXLEN];

	int i;
	int good_guess;
	int word_length;
	char hostname[MAXLEN];

	printf("Checking game session status...\n");
	// No more game slots available on the server
	if(gameSession == NULL)
	{
		sprintf(outbuf, "%s", "Connection failed no empty game slots on the server.");
		printf("Connection refused\n");
		//sendto(clientFileDescriptor, outbuf, strlen(outbuf) + 1, 0, (struct sockaddr*) &client.sender, client.sendsize);
		SendTo(clientFileDescriptor, outbuf, strlen(outbuf) + 1, 0, (struct sockaddr*) &client.sender, client.sendsize);
		return -1;
	}

	// If the client has just connected and this is the
	// beginning of the game.
	// 1. Send confirmation message for connection
	// 2. Pick a random word from the word list
	// 3. Store the random word in the game session
	if(gameSession->iSequenceNumber == 0)
	{
		// Set max number of lives the client has
		gameSession->iLives = MAX_LIVES;

		gethostname(hostname, MAXLEN);
		printf("sending confirmation message to the client..\n");
		sprintf(outbuf, "Playing hangman on host %s with %s:", hostname, gameSession->strUsername);
		SendTo(clientFileDescriptor, outbuf, strlen(outbuf) + 1, 0, (struct sockaddr*) &client.sender, client.sendsize);


		/* Pick a word at random from the list */
		printf("picking a random word...\n");
		srand((int) time((long *) 0)); /* randomize the seed */

		whole_word = word[rand() % NUM_OF_WORDS];
		word_length = strlen(whole_word);
		syslog(LOG_USER | LOG_INFO, "server chose hangman word %s", whole_word);

		/* No letters are guessed Initially */
		for (i = 0; i < word_length; i++)
			gameSession->strPartWord[i] = '-';

		gameSession->strPartWord[i] = '\0';

		// Send game session status to the client on start
		printf("sending game status to client..\n");
		sprintf(outbuf, "%s %d", gameSession->strPartWord, gameSession->iLives);
		printf("Game status: %s\n", outbuf);
		SendTo(clientFileDescriptor, outbuf, strlen(outbuf) + 1, 0, (struct sockaddr*) &client.sender, client.sendsize);


		// Store information in game session
		gameSession->cGameState = 'I';
		gameSession->strRandomWord = whole_word;
		gameSession->iRandomWordLength = word_length;

		// Increment the sequence number to indicate that the
		// game has started
		gameSession->iSequenceNumber++;

		printf("------------NEW GAME---------\n");
		PrintGameSession(gameSession);
		printf("-----------------------------\n");
		return 0;
	}

	// If the game has started and is Incomplete
	if(gameSession->cGameState == 'I')
	/* Get a letter from player guess */
	{
		printf("Processing guess for client. Printing game session...\n");
		PrintGameSession(gameSession);

		// If the client guesses right update the part word in the
		// game session. If the client is resuming their game skip the guess
		if(message[0] != ' ')
		{
			good_guess = 0;
			for (i = 0; i < gameSession->iRandomWordLength; i++) {
				if (message[0] == gameSession->strRandomWord[i]) {
					good_guess = 1;
					gameSession->strPartWord[i] = gameSession->strRandomWord[i];
				}
			}

			// If the client guesses wrong decrement their life
			if (!good_guess)
				gameSession->iLives--;
		}
		else
		{
			gethostname(hostname, MAXLEN);
			printf("sending confirmation message to the client..\n");
			sprintf(outbuf, "Playing hangman on host %s with %s:", hostname, gameSession->strUsername);
			SendTo(clientFileDescriptor, outbuf, strlen(outbuf) + 1, 0, (struct sockaddr*) &client.sender, client.sendsize);

		}


		// Check if the word has been guessed correctly
		if (strcmp(gameSession->strRandomWord, gameSession->strPartWord) == 0)
		{
			gameSession->cGameState = 'W';
			sprintf(outbuf, "Congratulations you won!\nSecret word: %s\nLives left: %d", gameSession->strPartWord, gameSession->iLives);
			printf("Client %s Won!\n", gameSession->strUsername);
			SendTo(clientFileDescriptor, outbuf, strlen(outbuf) + 1, 0, (struct sockaddr*) &client.sender, client.sendsize);


			return -1;
		}
		else if (gameSession->iLives == 0) {
			gameSession->cGameState = 'L';
			sprintf(outbuf, "Sorry you lost\nSecret word: %s", gameSession->strRandomWord);
			printf("Client %s Lost!\nSending final message: %s\n", gameSession->strUsername, outbuf);
			SendTo(clientFileDescriptor, outbuf, strlen(outbuf) + 1, 0, (struct sockaddr*) &client.sender, client.sendsize);

			return -1;
		}

		printf("sending game state to the client...\n");
		sprintf(outbuf, "%s %d", gameSession->strPartWord, gameSession->iLives);
		printf("%s\n", outbuf);
		SendTo(clientFileDescriptor, outbuf, strlen(outbuf) + 1, 0, (struct sockaddr*) &client.sender, client.sendsize);


		return 0;
	}

	return -1;
}

// Connection to peer using libsocket Connection()
int InitConnection(char *hostname, char *service, int type /* Client or Server */, int protocol /* UDP or TCP */)
{
	return Connection(hostname, service, type, protocol);
}

// Send message to peer using libsocket Send()
int SendMessage(int socketFileDescriptor, char* buffer, size_t size, int flags)
{
	return Send(socketFileDescriptor, buffer, size, flags);
}

// Receive message from peer using libsocket ReceiveFrom()
int ReceiveMessage(int iListenSocketFileDescriptor, char* buffer, int bufferSize, int flags , struct sockaddr *sender, socklen_t *sendsize)
{
	return ReceiveFrom(iListenSocketFileDescriptor, buffer, bufferSize, flags, sender, sendsize);
}

