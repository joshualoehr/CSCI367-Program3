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
	if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) != 0) {
		fprintf(stderr,"connect failed\n");
		return INVALID;
	}

	return sd;
}

int confirm_connection_allowed(ObserverState *state) {
	uint8_t response;
	if (recv(state->sd, &response, sizeof(uint8_t), NO_FLAGS) < SUCCESS) {
		fprintf(stderr, "Error: unable to recv connection confirmation from server.\n");
		return FAILURE;
	}

	return response == 'Y' ? SUCCESS : FAILURE;
}

int prompt_and_get_username(char *input) {
	fprintf(stdout, "Enter username: ");
	while (scanf("%s", input) < SUCCESS) {
		fprintf(stdout, "Enter username: ");
	}

	if (validate_username(input) == INVALID) {
		return prompt_and_get_username(input);
	} else {
		return SUCCESS;
	}
}

int validate_username(char *name) {
	int name_len = strlen(name);

	/* Check username is of valid length */
	if (0 == name_len || name_len > USERNAME_MAX_LENGTH) {
		fprintf(stdout, "Invalid: exceeded maximum of 10 characters\n");
		return INVALID;
	}

	/* Check username has only valid characters */
	for (int i = 0; i < name_len; i++) {
		if( (name[i] < '0' || name[i] > '9') && (name[i] < 'A' || name[i] > 'Z') &&
			(name[i] < 'a' || name[i] > 'z') && (name[i] != '_' )) {
			fprintf(stdout, "Invalid: must contain only alphanumeric characters and underscores\n");
			return INVALID;
		}
	}

	return SUCCESS;
}

int negotiate_username(ObserverState *state) {
	char input[255], response;

	while (1) {
		prompt_and_get_username(input);

		uint8_t input_len = strlen(input);
		if (send(state->sd, &input_len, sizeof(input_len), NO_FLAGS) < SUCCESS) {
			fprintf(stderr, "Error: unable to send username len to server (socket error).\n");
			return FAILURE;
		}

		if (send(state->sd, input, sizeof(uint8_t)*input_len, NO_FLAGS) < SUCCESS) {
			fprintf(stderr, "Error: unable to send username to server (socket error).\n");
			return FAILURE;
		}

		if (recv(state->sd, &response, sizeof(char), NO_FLAGS) < SUCCESS) {
			fprintf(stderr, "Error: unable to recv username negotiation from server (socket error).\n");
			return FAILURE;
		}

		switch (response) {
		case 'Y': return SUCCESS;
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

	if ((status = recv(state->sd, &net_order, sizeof(net_order), NO_FLAGS)) < 0) {
		fprintf(stderr, "Error: unable to recv message length (socket error).\n");
		return FAILURE;
	} else if (status == 0) {
		return INVALID;
	}
	msg_len = ntohs(net_order);

	char msg[msg_len + 2];
	if ((status = recv(state->sd, msg, sizeof(char) * msg_len, NO_FLAGS)) < 0) {
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

int main(int argc, char **argv) {
	int status, sd, port;

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

	if ((status = negotiate_username(&state)) == FAILURE) {
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
