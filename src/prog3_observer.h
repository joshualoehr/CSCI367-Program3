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
int main_observer(int argc, char **argv);

/* MOCKED FUNCTIONS */

extern int (*mock_connect)(int socket, const struct sockaddr *address, socklen_t address_len);
int mock_connect_success(int socket, const struct sockaddr *address, socklen_t address_len);
int mock_connect_failure(int socket, const struct sockaddr *address, socklen_t address_len);

extern int (*mock_prompt_and_get_username)(char *input);
int mock_prompt_and_get_username_valid(char *input);
int mock_prompt_and_get_username_invalid(char *input);

extern int (*mock_send_username)(int sockfd, const void *buf, size_t len, int flags);
int mock_send_username_success(int sockfd, const void *buf, size_t len, int flags);

extern int (*mock_send_username_len)(int sockfd, const void *buf, size_t len, int flags);
int mock_send_username_success_len(int sockfd, const void *buf, size_t len, int flags);

extern int (*mock_recv_negotiation)(int sockfd, void *buf, size_t len, int flags);
int mock_recv_negotiation_timeout(int sockfd, void *buf, size_t len, int flags);
int mock_recv_negotiation_Y(int sockfd, void *buf, size_t len, int flags);
int mock_recv_negotiation_I(int sockfd, void *buf, size_t len, int flags);
int mock_recv_negotiation_T(int sockfd, void *buf, size_t len, int flags);

#endif /* PROG3_OBSERVER_H_ */
