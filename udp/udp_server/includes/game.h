#ifndef INCLUDES_GAME_H_
#define INCLUDES_GAME_H_

#include "../../../libsocket/socket.h"

typedef struct GameSession{
	char cGameState;
	char* strUsername;
} GameSession;

#define MAX_GAME_SESSIONS 50

void play_hangman(int in, int out, struct Address client, GameSession* gameSession);
void ProcessRequest(int in, int out, struct Address client, GameSession* gameSession);
int findGameSession(GameSession* gameSessions, int len, char* username);


#endif /* INCLUDES_GAME_H_ */