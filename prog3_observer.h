/*
 * prog3_observer.h
 *
 *  Created on: Feb 26, 2017
 *      Author: loehrj
 */

#ifndef PROG3_OBSERVER_H_
#define PROG3_OBSERVER_H_

#define INVALID -1
#define FAILURE 0
#define SUCCESS 1

#define MIN_PORT 1024
#define NO_FLAGS 0

#define USERNAME_MAX_LENGTH 10
#define MSG_MAX_LEN 1000

#include <sys/socket.h>

typedef struct ObserverState {
	int sd;
	int name_len;
	char name[10];
} ObserverState;

int init_connection(char *host, int port);
int confirm_connection_allowed(ObserverState *state);
int prompt_and_get_username(char *input);
int validate_username(char *name);
int negotiate_username(ObserverState *state);
int recv_message(ObserverState *state);
int main(int argc, char **argv);

#endif /* PROG3_OBSERVER_H_ */
