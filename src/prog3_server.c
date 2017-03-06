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
	state->pending_count = 0;

	state->p_conns = (struct Connection**) malloc(sizeof(struct Connection*) * MAX_PARTICIPANTS);
	for (int i = 0; i < MAX_PARTICIPANTS; i++) {
		state->p_conns[i] = (struct Connection*) malloc(sizeof(struct Connection));
	}
	state->o_conns = (struct Connection**) malloc(sizeof(struct Connection*) * MAX_OBSERVERS);
	for (int i = 0; i < MAX_OBSERVERS; i++) {
		state->o_conns[i] = (struct Connection*) malloc(sizeof(struct Connection));
	}
	state->pending_conns = (struct Connection**) malloc(sizeof(struct Connection*) * MAX_OBSERVERS);
	for (int i = 0; i < MAX_PENDING; i++) {
		state->pending_conns[i] = (struct Connection*) malloc(sizeof(struct Connection));
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


int negotiate_connection(int l_sd, ServerState *state) {
	if (state->pending_count >= MAX_PENDING) {
		fprintf(stdout, "Connection declined: too many pending connections already.\n");
		return INVALID;
	}

	if (new_connection(l_sd, state) == FAILURE) {
		return FAILURE;
	}

	Connection *pending_conn = state->pending_conns[state->pending_count++];
	FD_SET(pending_conn->sd, &state->master_set);
	state->fd_max = (pending_conn->sd > state->fd_max) ? pending_conn->sd : state->fd_max;

	fprintf(stdout, "Established new connection (%d)\n", pending_conn->sd);

	char response = 'Y';
	if ( (l_sd == state->p_listener && state->p_count >= MAX_PARTICIPANTS) ||
		 (l_sd == state->o_listener && state->o_count >= MAX_OBSERVERS) ) {
		response = 'N';
	}

	if (send(pending_conn->sd, &response, sizeof(char), NO_FLAGS) < SUCCESS) {
		fprintf(stderr, "Error: unable to send connection confirmation to participant (socket error).\n");
		return FAILURE;
	}

	return SUCCESS;
}


int new_connection(int l_sd, ServerState *state) {
	Connection *pending_conn = state->pending_conns[state->pending_count];
	pending_conn->type = (l_sd == state->p_listener) ? PARTICIPANT : OBSERVER;
	pending_conn->sd = INVALID;
	pending_conn->name_len = 0;
	memset(pending_conn->name, '\0', USERNAME_MAX_LENGTH + 1);

	struct sockaddr_in cad;
	int alen = sizeof(cad);
	pending_conn->sd = ACCEPT(l_sd, (struct sockaddr *)&cad, &alen);
	if (pending_conn->sd == INVALID) {
		fprintf(stderr, "Error: socket accept failed\n");
		return FAILURE;
	}

	return SUCCESS;
}


int validate_username(char *name, int name_len, ServerState *state) {
	/* Check username is of valid length */
	if (0 == name_len || name_len > USERNAME_MAX_LENGTH) {
		return 'I';
	}

	/* Check username has only valid characters */
	for (int i = 0; i < name_len; i++) {
		if( (name[i] < '0' || name[i] > '9') && (name[i] < 'A' || name[i] > 'Z') &&
			(name[i] < 'a' || name[i] > 'z') && (name[i] != '_' )) {
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
			if (negotiate_connection(state.p_listener, &state) == FAILURE) {
				fprintf(stderr, "Error: unable to negotiate connection with participant.\n");
			}
		}
		if (FD_ISSET(state.o_listener, &state.read_set)) {
			/* Send connection confirmation */
			if (negotiate_connection(state.o_listener, &state) == FAILURE) {
				fprintf(stderr, "Error: unable to negotiate connection with observer.\n");
			}
		}

		/* Check pending connections */
		for (int i = 0; i < state.pending_count; i++) {
			Connection *pending_conn = state.pending_conns[i];

			if (FD_ISSET(pending_conn->sd, &state.read_set)) {
				char response;

				/* Receive username (length, then string) */
				fprintf(stdout, "Recving username len (%d)... ", pending_conn->sd);
				uint16_t name_len = 0;
				uint16_t net_order = 0;
				if (recv(pending_conn->sd, &net_order, sizeof(net_order), NO_FLAGS) < SUCCESS) {
					fprintf(stderr, "Error: unable to recv username len from participant (socket error).\n");
				}
				name_len = ntohs(net_order);
				fprintf(stdout, "%d -> %d\n", net_order, name_len);

				fprintf(stdout, "Recving username (%d)... ", pending_conn->sd);
				char name[name_len + 1];
				if (recv(pending_conn->sd, name, sizeof(name), NO_FLAGS) < SUCCESS) {
					fprintf(stderr, "Error: unable to recv username from participant (socket error).\n");
				}
				name[name_len] = '\0';
				fprintf(stdout, "%s\n", name);

				/* Send username confirmation */
				response = validate_username(name, name_len, &state);
				fprintf(stdout, "Sending username confirmation (%c)\n", response);
				if (send(pending_conn->sd, &response, sizeof(char), NO_FLAGS) < SUCCESS) {
					fprintf(stderr, "Error: unable to send username confirmation to participant (socket error).\n");
				}

				if (response == 'Y') {
					pending_conn->name_len = name_len;
					strcpy(pending_conn->name, name);

					// broadcast to observers
					fprintf(stdout, "Adding active participant (%s) to p_conns\n", pending_conn->name);
					state.p_conns[state.p_count++] = pending_conn;

					/* Remove connection from pending_conns */
					for (int j = 0; j < state.pending_count; j++) {
						if (state.pending_conns[j] == pending_conn) {
							while (j < state.pending_count - 1) {
								state.pending_conns[j] = state.pending_conns[j+1];
								j++;
							}
							break;
						}
					}
					state.pending_count--;
					i--;
				}
			}
		}

		/* Check participant connections */
		for (int i = 0; i < state.p_count; i++) {
			Connection *conn = state.p_conns[i];

			if (FD_ISSET(conn->sd, &state.read_set)) {
//				fprintf(stdout, "Participant sending!");
				// handle participant communication
			}
			if (FD_ISSET(conn->sd, &state.write_set)) {
//				fprintf(stdout, "Participant recving!");
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
		for (int p = 0; p < state.pending_count; p++) {
			FD_SET(state.pending_conns[p]->sd, &state.master_set);
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
