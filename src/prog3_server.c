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
	state->p_conns = (struct Connection**) malloc(sizeof(struct Connection*) * MAX_PARTICIPANTS);
	for (int i = 0; i < MAX_PARTICIPANTS; i++) {
		state->p_conns[i] = (struct Connection*) malloc(sizeof(struct Connection));
	}
	state->o_conns = (struct Connection**) malloc(sizeof(struct Connection*) * MAX_OBSERVERS);
	for (int i = 0; i < MAX_OBSERVERS; i++) {
		state->o_conns[i] = (struct Connection*) malloc(sizeof(struct Connection));
	}

	state->pending_conn = (struct Connection*) malloc(sizeof(struct Connection));

	state->master_set = (fd_set*) malloc(sizeof(fd_set));
	state->read_set = (fd_set*) malloc(sizeof(fd_set));
	state->write_set = (fd_set*) malloc(sizeof(fd_set));
	FD_ZERO(state->master_set);
	FD_ZERO(state->read_set);
	FD_ZERO(state->write_set);

	// TODO: This will change
	state->timeout.tv_sec = 0;
	state->timeout.tv_usec = 500000;
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

	return sd;
}

char handle_listener(int l_sd, ServerState *state) {
	if (new_connection(l_sd, state) == FAILURE) {
		return '!';
	}

	if (state->p_count >= MAX_PARTICIPANTS || state->o_count >= MAX_OBSERVERS) {
		close(state->pending_conn->sd);
		return 'N';
	}

	state->p_conns[state->p_count++] = state->pending_conn;
	return 'Y';
}

int new_connection(int l_sd, ServerState *state) {
	state->pending_conn->type = INVALID;
	state->pending_conn->sd = INVALID;
	state->pending_conn->name_len = 0;
	state->pending_conn->name = "";

	struct sockaddr_in cad;
	int alen = sizeof(cad);
	int new_sd = ACCEPT(l_sd, (struct sockaddr *)&cad, &alen);
	if (new_sd == INVALID) {
		fprintf(stderr, "Error: socket accept failed\n");
		return FAILURE;
	}

	state->pending_conn->type = (l_sd == state->p_listener) ? PARTICIPANT : OBSERVER;
	state->pending_conn->sd = new_sd;

	return SUCCESS;
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
		fprintf(stderr, "Error: invalid port %s", argv[1]);
		return EXIT_FAILURE;
	}
	if ((state.o_listener = init_listener(o_port)) == INVALID) {
		fprintf(stderr, "Error: invalid port %s", argv[2]);
		return EXIT_FAILURE;
	}
	FD_SET(state.p_listener, state.master_set);
	FD_SET(state.o_listener, state.master_set);
	state.fd_max = (state.p_listener > state.o_listener) ? state.p_listener : state.o_listener;

	while (1) {
		/* Select sockets and update fd_sets */
		state.read_set = state.master_set;
		state.write_set = state.master_set;
		if (select(state.fd_max + 1, state.read_set, state.write_set, NULL, &state.timeout) == INVALID) {
			fprintf(stderr, "Error: unable to select.\n");
			return EXIT_FAILURE;
		}

		/* Check participant connections */
		for (int i = 0; i < state.p_count; i++) {
			fprintf(stdout, "%d\n", i);
			Connection *conn = state.p_conns[i];

			if (FD_ISSET(conn->sd, state.read_set)) {
				fprintf(stdout, "Participant connected!");
				// handle participant communication
			}
		}

		/* Check observer connections */
		for (int i = 0; i < state.o_count; i++) {
			fprintf(stdout, "%d\n", i);
			Connection *conn = state.o_conns[i];

			if (FD_ISSET(conn->sd, state.write_set)) {
				fprintf(stdout, "Observer connected!");
				// handle observer communication
			}
		}

		/* Check listeners */
		if (FD_ISSET(state.p_listener, state.read_set)) {
			fprintf(stdout, "Handling p_listener\n");
			char response = handle_listener(state.p_listener, &state);
			send(state.pending_conn->sd, &response, sizeof(char), NO_FLAGS);
		}
		if (FD_ISSET(state.o_listener, state.read_set)) {
			fprintf(stdout, "Handling o_listener\n");
			char response = handle_listener(state.o_listener, &state);
			send(state.pending_conn->sd, &response, sizeof(char), NO_FLAGS);
		}

		for (int i = 0; i < state.fd_max; i++) {
			if (FD_ISSET(i, state.read_set) || FD_ISSET(i, state.write_set)) {
				fprintf(stdout, "%d ready\n", i);
			}
		}

		FD_ZERO(state.master_set);
		FD_SET(state.p_listener, state.master_set);
		FD_SET(state.o_listener, state.master_set);
		for (int p = 0; p < state.p_count; p++) {
			FD_SET(state.p_conns[p]->sd, state.master_set);
		}
		for (int o = 0; o < state.o_count; o++) {
			FD_SET(state.o_conns[o]->sd, state.master_set);
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
