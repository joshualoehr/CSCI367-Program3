#ifdef TEST
#define CONNECT mock_connect
#define PROMPT_AND_GET_USERNAME mock_prompt_and_get_username
#define SEND_USERNAME mock_send_username
#define RECV_NEGOTIATION mock_recv_negotiation
#else
#define CONNECT connect
#define PROMPT_AND_GET_USERNAME prompt_and_get_username
#define SEND_USERNAME send
#define RECV_NEGOTIATION recv
#endif

#include "prog3_participant.h"

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

	fprintf(stdout, "Able to connect socket %d on port %d\n", sd, port);

	return sd;
}

// Untested
int confirm_connection_allowed(ParticipantState *state) {
	uint8_t response;
	fprintf(stdout, "Waiting for server confirmation...\n");
	if (recv(state->sd, &response, sizeof(uint8_t), NO_FLAGS) < SUCCESS) {
		fprintf(stderr, "Error: unable to recv connection confirmation from server.\n");
		return FAILURE;
	}
	fprintf(stdout, "Server responded with %c\n", response);

	return response == 'Y' ? SUCCESS : FAILURE;
}

// Untested
int prompt_and_get_username(char *input) {
	fprintf(stdout, "Enter username: ");
	while (scanf("%s", input) < SUCCESS) {
		fprintf(stderr, "Error: unable to read stdin, try again.\n");
	}
	fprintf(stdout, "\n");

	return SUCCESS;
}

int validate_username(char *name) {
	int len = strlen(name);
	if (0 == len || len > USERNAME_MAX_LENGTH) {
		return INVALID;
	}

	for (int i = 0; i < len; i++) {
		char c = name[i];
		if( (c < '0' || c > '9') && (c < 'A' || c > 'Z') &&
			(c < 'a' || c > 'z') && (c != '_' )) {
			  return INVALID;
		}
	}

	return SUCCESS;
}

int negotiate_username(ParticipantState *state) {
	char input[255], response;

	while (1) {
		PROMPT_AND_GET_USERNAME(input);
		if (SEND_USERNAME(state->sd, input, sizeof(char)*strlen(input), NO_FLAGS) < SUCCESS) {
			fprintf(stderr, "Error: unable to send username to server.\n");
			return FAILURE;
		}
		if (RECV_NEGOTIATION(state->sd, &response, sizeof(char), NO_FLAGS)) {
			fprintf(stderr, "Error: unable to recv username negotiation from server.\n");
			return FAILURE;
		}
		switch (response) {
		case 'Y': return SUCCESS;
		case 'I': return INVALID;
		case 'T': return INVALID;
		default: fprintf(stderr, "Error: unexpected username negotiation response (%c)", response);
				 return FAILURE;
		}
	}

	return FAILURE;
}

int main_participant(int argc, char **argv) {
	int sd, port;

	if ( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"prog3_participant server_address server_port\n");
		return EXIT_FAILURE;
	}

	/* Initialize client socket */
	port = atoi(argv[2]);
	if ( (sd = init_connection(argv[1], port)) == INVALID ) {
		return EXIT_FAILURE;
	}

	/* Initialize state */
	ParticipantState state;
	state.sd = sd;

	/* Confirm server has room for another connection */
	if (confirm_connection_allowed(&state) == FAILURE) {
		close(state.sd);
		return EXIT_SUCCESS;
	}

	negotiate_username(&state);





	return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
	return main_participant(argc, argv);
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

#pragma GCC diagnostic pop
