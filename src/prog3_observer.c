#define DEBUG 0

#ifdef TEST
#define CONNECT mock_connect
#define PROMPT_AND_GET_USERNAME mock_prompt_and_get_username
#define SEND_USERNAME_LEN mock_send_username_len
#define SEND_USERNAME mock_send_username
#define RECV_NEGOTIATION mock_recv_negotiation
#else
#define CONNECT connect
#define PROMPT_AND_GET_USERNAME prompt_and_get_username
#define SEND_USERNAME_LEN send
#define SEND_USERNAME send
#define RECV_NEGOTIATION recv
#endif

#include "prog3_observer.h"

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

int init_connection(char *host, int port) {
	int sd;
	struct hostent *ptrh; /* pointer to a host table entry */
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold an IP address */

	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet */

	if (port > MIN_PORT) /* test for legal value */
		sad.sin_port = htons((u_short)port);
	else {
		fprintf(stderr,"Error: bad port number %d\n",port);
		return INVALID;
	}

	/* Convert host name to equivalent IP address and copy to sad. */
	ptrh = gethostbyname(host);
	if ( ptrh == NULL ) {
		fprintf(stderr,"Error: Invalid host: %s\n", host);
		return INVALID;
	}

	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

	/* Map TCP transport protocol name to protocol number. */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		return INVALID;
	}

	/* Create a socket. */
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		return INVALID;
	}

	/* Connect the socket to the specified server. */
	if (CONNECT(sd, (struct sockaddr *)&sad, sizeof(sad)) != 0) {
		fprintf(stderr,"connect failed\n");
		return INVALID;
	}

	if (DEBUG) fprintf(stdout, "Connected socket %d on port %d\n", sd, port);

	return sd;
}

// Untested
int confirm_connection_allowed(ObserverState *state) {
	uint8_t response;
	if (DEBUG) fprintf(stdout, "Waiting for server confirmation... ");
	if (recv(state->sd, &response, sizeof(uint8_t), NO_FLAGS) < SUCCESS) {
		fprintf(stderr, "Error: unable to recv connection confirmation from server.\n");
		return FAILURE;
	}
	if (DEBUG) fprintf(stdout, "%c\n", response);

	return response == 'Y' ? SUCCESS : FAILURE;
}

// Untested
int prompt_and_get_username(char *input) {
	fprintf(stdout, "Enter username: ");
	while (scanf("%s", input) < SUCCESS) {
		fprintf(stderr, "Error: unable to read stdin, try again.\n");
	}

	return SUCCESS;
}

int negotiate_username(ObserverState *state) {
	char input[255], response;

	while (1) {
		PROMPT_AND_GET_USERNAME(input);

		uint8_t input_len = strlen(input);
		if (DEBUG) fprintf(stdout, "Sending username len (%d)... \n", input_len);
		if (SEND_USERNAME_LEN(state->sd, &input_len, sizeof(input_len), NO_FLAGS) < SUCCESS) {
			fprintf(stderr, "Error: unable to send username len to server (socket error).\n");
			return FAILURE;
		}

		if (DEBUG) fprintf(stdout, "Sending username (%s)... \n", input);
		if (SEND_USERNAME(state->sd, input, sizeof(uint8_t)*input_len, NO_FLAGS) < SUCCESS) {
			fprintf(stderr, "Error: unable to send username to server (socket error).\n");
			return FAILURE;
		}

		if (DEBUG) fprintf(stdout, "Recving username negotiation...\n");
		if (RECV_NEGOTIATION(state->sd, &response, sizeof(char), NO_FLAGS) < SUCCESS) {
			fprintf(stderr, "Error: unable to recv username negotiation from server (socket error).\n");
			return FAILURE;
		}
		switch (response) {
		case 'Y': if (DEBUG) fprintf(stdout, "Server accepted username (%s)!\n", input); return SUCCESS;
		case 'I': fprintf(stdout, "Username %s invalid, please try again.\n", input); break;
		case 'T': fprintf(stdout, "Username %s already affiliated, please try again.\n", input); break;
		case 'N': fprintf(stdout, "Username %s does not exist.\n", input); return INVALID;
		default: fprintf(stderr, "Error: unexpected username negotiation response (%c)", response);
				 return FAILURE;
		}
	}

	return FAILURE;
}

int recv_message(ObserverState *state) {
	int status;
	uint16_t msg_len;
	uint16_t net_order;

	if (DEBUG) fprintf(stdout, "Recving msg len... ");
	status = recv(state->sd, &net_order, sizeof(net_order), NO_FLAGS);
	if (status < 0) {
		fprintf(stderr, "Error: unable to recv message length (socket error).\n");
		return FAILURE;
	} else if (status == 0) {
		return INVALID;
	}
	msg_len = ntohs(net_order);
	if (DEBUG) fprintf(stdout, "(%d -> %d)\n", net_order, msg_len);

	if (DEBUG) fprintf(stdout, "Recving msg...\n");
	char msg[msg_len + 2];
//	memset(msg, '\0', msg_len);
	status = recv(state->sd, msg, sizeof(char) * msg_len, NO_FLAGS);
	if (status < 0) {
		fprintf(stderr, "Error: unable to recv message (socket error).\n");
		return FAILURE;
	} else if (status == 0) {
		return INVALID;
	}
	msg[msg_len + 1] = '\0';

	if (msg[msg_len-1] != '\n') {
		msg[msg_len] = '\n';
	}

	fprintf(stdout, "%s", msg);
	return SUCCESS;
}

int main_observer(int argc, char **argv) {
	int sd, port;

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"prog3_observer server_address server_port\n");
		return EXIT_FAILURE;
	}

	/* Initialize client socket */
	port = atoi(argv[2]);
	if ( (sd = init_connection(argv[1], port)) == INVALID ) {
		return EXIT_FAILURE;
	}

	/* Initialize state */
	ObserverState state;
	state.sd = sd;

	/* Confirm server has room for another connection */
	if (confirm_connection_allowed(&state) == FAILURE) {
		close(state.sd);
		return EXIT_SUCCESS;
	}

	int status = negotiate_username(&state);
	if (status == FAILURE) {
		fprintf(stderr, "Error: unable to negotiate username.\n");
		return EXIT_FAILURE;
	} else if (status == INVALID) {
		return EXIT_SUCCESS;
	}

	while (1) {
		status = recv_message(&state);
		if (status == FAILURE) {
			return EXIT_FAILURE;
		} else if (status == INVALID) {
			return EXIT_SUCCESS;
		}
	}

	return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
	return main_observer(argc, argv);
}

/* MOCKED FUNCTIONS */

/* connect to server */
int (*mock_connect)(int socket, const struct sockaddr *address, socklen_t address_len) = connect;
int mock_connect_success(int socket, const struct sockaddr *address, socklen_t address_len) {
	return SUCCESS;
}
int mock_connect_failure(int socket, const struct sockaddr *address, socklen_t address_len) {
	return INVALID;
}

/* get username input */
int (*mock_prompt_and_get_username)(char *input) = prompt_and_get_username;
int mock_prompt_and_get_username_valid(char *input) {
	input = "validname";
	return SUCCESS;
}
int mock_prompt_and_get_username_invalid(char *input) {
	input = "invalid name";
	return SUCCESS;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

/* send username len to server */
int (*mock_send_username_len)(int sockfd, const void *buf, size_t len, int flags) = send;
int mock_send_username_len_success(int sockfd, const void *buf, size_t len, int flags) {
	return SUCCESS;
}

/* send username to server */
int (*mock_send_username)(int sockfd, const void *buf, size_t len, int flags) = send;
int mock_send_username_success(int sockfd, const void *buf, size_t len, int flags) {
	return SUCCESS;
}

/* recv username negotation from server */
int (*mock_recv_negotiation)(int sockfd, void *buf, size_t len, int flags) = recv;
int mock_recv_negotiation_timeout(int sockfd, void *buf, size_t len, int flags) {
	return FAILURE;
}
int mock_recv_negotiation_Y(int sockfd, void *buf, size_t len, int flags) {
	char val = 'Y';
	memcpy(buf, &val, sizeof(char));
	return SUCCESS;
}
int mock_recv_negotiation_I(int sockfd, void *buf, size_t len, int flags) {
	char val = 'I';
	memcpy(buf, &val, sizeof(char));
	return SUCCESS;
}
int mock_recv_negotiation_T(int sockfd, void *buf, size_t len, int flags) {
	char val = 'T';
	memcpy(buf, &val, sizeof(char));
	return SUCCESS;
}
