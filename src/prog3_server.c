#ifdef TEST
#define ACCEPT mock_accept
#else
#define ACCEPT accept
#endif

#include "prog3_server.h"

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

int (*mock_accept)(int sockfd, struct sockaddr *addr, socklen_t *addrlen) = accept;

void init_server_state(ServerState *state) {
	state->p_count = 0;
	state->o_count = 0;

	state->p_conns = (struct Connection**) malloc(sizeof(struct Connection*) * MAX_PARTICIPANTS);
	for (int i = 0; i < MAX_PARTICIPANTS; i++) {
		state->p_conns[i] = (struct Connection*) malloc(sizeof(struct Connection));
	}
	state->o_conns = (struct Connection**) malloc(sizeof(struct Connection*) * MAX_OBSERVERS);
	for (int i = 0; i < MAX_OBSERVERS; i++) {
		state->o_conns[i] = (struct Connection*) malloc(sizeof(struct Connection));
	}

	FD_ZERO(&state->master_set);
	FD_ZERO(&state->read_set);
	FD_ZERO(&state->write_set);

	// TODO: This will change
	state->timeout.tv_sec = 0;
	state->timeout.tv_usec = 50000;
}


int init_listener(int port) {
	int sd;

	if (port <= MIN_PORT) {
		return INVALID;
	}

	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold server's address */
	int optval = 1; /* boolean value when we set socket option */

	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet */
	sad.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */

	sad.sin_port = htons((u_short)port);

	/* Map TCP transport protocol name to protocol number */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		return INVALID;
	}

	/* Create a socket */
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		return INVALID;
	}

	/* Allow reuse of port - avoid "Bind failed" issues */
	if( setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) {
		fprintf(stderr, "Error Setting socket option failed\n");
		return INVALID;
	}

	/* Bind a local address to the socket */
	if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		fprintf(stderr,"Error: Bind failed\n");
		return INVALID;
	}

	/* Specify size of request queue */
	if (listen(sd, QLEN) < 0) {
		fprintf(stderr,"Error: Listen failed\n");
		return INVALID;
	}

	fprintf(stdout, "Socket %d listening on port %d...\n", sd, port);
	return sd;
}


char handle_listener(int l_sd, ServerState *state) {
	fprintf(stdout, "Handling listener (%d)...\n", l_sd);
	if (new_connection(l_sd, state) == FAILURE) {
		return '!';
	}

	if ( (l_sd == state->p_listener && state->p_count >= MAX_PARTICIPANTS) ||
		 (l_sd == state->o_listener && state->o_count >= MAX_OBSERVERS) ) {
		return 'N';
	}

	return 'Y';
}


int new_connection(int l_sd, ServerState *state) {
	state->pending_conn = (struct Connection*) malloc(sizeof(struct Connection));
	state->pending_conn->type = INVALID;
	state->pending_conn->sd = INVALID;
	state->pending_conn->name_len = 0;
	state->pending_conn->name = "";

	struct sockaddr_in cad;
	int alen = sizeof(cad);
	fprintf(stdout, "Waiting to accept new connection (%d)... ", l_sd);
	int new_sd = ACCEPT(l_sd, (struct sockaddr *)&cad, &alen);
	if (new_sd == INVALID) {
		fprintf(stderr, "Error: socket accept failed\n");
		return FAILURE;
	}
	fprintf(stdout, "accepted (%d)!\n", new_sd);

	state->pending_conn->type = (l_sd == state->p_listener) ? PARTICIPANT : OBSERVER;
	state->pending_conn->sd = new_sd;

	return SUCCESS;
}


int validate_username(int len, char *name, ServerState *state) {
	/* Check username is of valid length */
	if (0 == len || len > USERNAME_MAX_LENGTH) {
		return 'I';
	}

	/* Check username has only valid characters */
	for (int i = 0; i < len; i++) {
		char c = name[i];
		if( (c < '0' || c > '9') && (c < 'A' || c > 'Z') &&
			(c < 'a' || c > 'z') && (c != '_' )) {
			  return 'I';
		}
	}

	/* Check username is not yet taken */
	for (int i = 0; i < state->p_count; i++) {
		if (strcmp(state->p_conns[i]->name, name) == 0) {
			return 'T';
		}
	}

	return 'Y';
}


int main_server(int argc, char **argv) {
	int p_port, o_port;

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"prog3_server server_participant_port server_observer_port\n");
		return EXIT_FAILURE;
	}

	/* Initialize server state */
	ServerState state;
	init_server_state(&state);

	p_port = atoi(argv[1]);
	o_port = atoi(argv[2]);

	/* Initialize listeners */
	if ((state.p_listener = init_listener(p_port)) == INVALID) {
		fprintf(stderr, "Error: unable to initialize listener on port %s\n", argv[1]);
		return EXIT_FAILURE;
	}
	if ((state.o_listener = init_listener(o_port)) == INVALID) {
		fprintf(stderr, "Error: unable to initialize listener on port %s\n", argv[2]);
		return EXIT_FAILURE;
	}
	FD_SET(state.p_listener, &state.master_set);
	FD_SET(state.o_listener, &state.master_set);
	state.fd_max = (state.p_listener > state.o_listener) ? state.p_listener : state.o_listener;

	while (1) {
		/* Select sockets and update fd_sets */
		state.read_set = state.master_set;
		state.write_set = state.master_set;
		if (select(state.fd_max + 1, &state.read_set, &state.write_set, NULL, &state.timeout) == INVALID) {
			fprintf(stderr, "Error: unable to select.\n");
			return EXIT_FAILURE;
		}

		/* Check listeners */
		if (FD_ISSET(state.p_listener, &state.read_set)) {
			/* Send connection confirmation */
			char response = handle_listener(state.p_listener, &state);
			if (send(state.pending_conn->sd, &response, sizeof(char), NO_FLAGS) < SUCCESS) {
				fprintf(stderr, "Error: unable to send connection confirmation to participant (socket error).\n");
			}

			fprintf(stdout, "Recving username len... ");
			/* Receive username (length, then string) */
			uint8_t name_len;
			if (recv(state.pending_conn->sd, &name_len, sizeof(uint8_t), NO_FLAGS) < SUCCESS) {
				fprintf(stderr, "Error: unable to recv username len from participant (socket error).\n");
			}
			fprintf(stdout, "%d\n", name_len);
			char name[name_len];
			fprintf(stdout, "Recving username... ");
			if (recv(state.pending_conn->sd, &name, sizeof(char) * name_len, NO_FLAGS) < SUCCESS) {
				fprintf(stderr, "Error: unable to recv username from participant (socket error).\n");
			}
			fprintf(stdout, "%s\n", name);

			/* Send username confirmation */
			response = validate_username(name_len, name, &state);
			fprintf(stdout, "Sending username confirmation (%c)...\n", response);
			if (send(state.pending_conn->sd, &response, sizeof(char), NO_FLAGS) < SUCCESS) {
				fprintf(stderr, "Error: unable to send username confirmation to participant (socket error).\n");
			} else {
				state.pending_conn->name = name;
				state.pending_conn->name_len = name_len;
			}

			if (response == 'Y') {
				// broadcast to observers
				fprintf(stdout, "Adding active participant (%s) to p_conns\n", name);
				state.p_conns[state.p_count++] = state.pending_conn;
			}
		}
		if (FD_ISSET(state.o_listener, &state.read_set)) {
			fprintf(stdout, "Handling o_listener\n");
			char response = handle_listener(state.o_listener, &state);
			send(state.pending_conn->sd, &response, sizeof(char), NO_FLAGS);
		}

		/* Check participant connections */
		for (int i = 0; i < state.p_count; i++) {
			Connection *conn = state.p_conns[i];

			if (FD_ISSET(conn->sd, &state.read_set)) {
				fprintf(stdout, "Participant sending!");
				// handle participant communication
			}
			if (FD_ISSET(conn->sd, &state.write_set)) {
				fprintf(stdout, "Participant recving!");
				// handle participant communication
			}
		}

		/* Check observer connections */
		for (int i = 0; i < state.o_count; i++) {
			Connection *conn = state.o_conns[i];

			if (FD_ISSET(conn->sd, &state.read_set)) {
				fprintf(stdout, "Observer sending!");
				// handle observer communication
			}
			if (FD_ISSET(conn->sd, &state.write_set)) {
				fprintf(stdout, "Observer recving!");
				// handle observer communication
			}
		}

		FD_ZERO(&state.master_set);
		FD_SET(state.p_listener, &state.master_set);
		FD_SET(state.o_listener, &state.master_set);
		for (int p = 0; p < state.p_count; p++) {
			FD_SET(state.p_conns[p]->sd, &state.master_set);
		}
		for (int o = 0; o < state.o_count; o++) {
			FD_SET(state.o_conns[o]->sd, &state.master_set);
		}
	}
}

int main(int argc, char** argv) {
	return main_server(argc, argv);
}


/* MOCKED FUNCTIONS */

int mock_accept_success(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	return 1000;
}
