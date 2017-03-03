/*
 * prog3_participant.h
 *
 *  Created on: Feb 26, 2017
 *      Author: loehrj
 */

#ifndef PROG3_PARTICIPANT_H_
#define PROG3_PARTICIPANT_H_

#define INVALID -1
#define FAILURE 0
#define SUCCESS 1

#define MIN_PORT 1024
#define NO_FLAGS 0

#define USERNAME_MAX_LENGTH 10

#include <sys/socket.h>

typedef struct ParticipantState {
	int sd;
	int name_len;
	char name[10];
} ParticipantState;

int init_connection(char *host, int port);
int confirm_connection_allowed(ParticipantState *state);
int prompt_and_get_username(char *input);
int validate_username(char *name);
int negotiate_username(ParticipantState *state);
int main_participant(int argc, char **argv);


/* MOCKED FUNCTIONS */

extern int (*mock_connect)(int socket, const struct sockaddr *address, socklen_t address_len);
int mock_connect_success(int socket, const struct sockaddr *address, socklen_t address_len);
int mock_connect_failure(int socket, const struct sockaddr *address, socklen_t address_len);

extern int (*mock_prompt_and_get_username)(char *input);
int mock_prompt_and_get_username_valid(char *input);
int mock_prompt_and_get_username_invalid(char *input);

extern int (*mock_send_username)(int sockfd, const void *buf, size_t len, int flags);
int mock_send_username_success(int sockfd, const void *buf, size_t len, int flags);

extern int (*mock_recv_negotiation)(int sockfd, void *buf, size_t len, int flags);
int mock_recv_negotiation_timeout(int sockfd, void *buf, size_t len, int flags);
int mock_recv_negotiation_Y(int sockfd, void *buf, size_t len, int flags);
int mock_recv_negotiation_I(int sockfd, void *buf, size_t len, int flags);
int mock_recv_negotiation_T(int sockfd, void *buf, size_t len, int flags);



#endif /* PROG3_PARTICIPANT_H_ */
