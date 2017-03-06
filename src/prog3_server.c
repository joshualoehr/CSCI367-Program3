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

/* HELPER FUNCTIONS */

int recv_(Connection *conn, void *buf, size_t len, int flags, ServerState *state) {
	int status = recv(conn->sd, buf, len, flags);
	if (status == 0) {
		return INVALID;
	} else if (status < 0) {
		fprintf(stderr, "Error: unable to recv message (socket error)\n");
		return FAILURE;
	}

	return SUCCESS;
}

int remove_connection(Connection *conn, ServerState *state) {
	/* Remove connection from pending_conns */
	fprintf(stdout, "Removing connection (%s|%d)\n", conn->name, conn->sd);

	Connection **conns;
	int *conn_count;

	switch(conn->type) {
	case PARTICIPANT: conns = state->p_conns; conn_count = &state->p_count; break;
	case OBSERVER: conns = state->o_conns; conn_count = &state->o_count; break;
	case PENDING_PARTICIPANT:
	case PENDING_OBSERVER: conns = state->pending_conns; conn_count = &state->pending_count; break;
	}
	for (int j = 0; j < *conn_count; j++) {
		if (conns[j] == conn) {
			while (j < *conn_count - 1) {
				conns[j] = conns[j+1];
				j++;
			}
			break;
		}
		if (j == *conn_count - 1) {
			fprintf(stderr, "Error: Connection (%s|%d) not found.\n", conn->name, conn->sd);
			return FAILURE;
		}
	}

	close(conn->sd);
	*conn_count = *conn_count - 1;
	return SUCCESS;
}

/* MAIN FUNCTIONS */

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
	pending_conn->type = (l_sd == state->p_listener) ? PENDING_PARTICIPANT : PENDING_OBSERVER;
	pending_conn->sd = INVALID;
	pending_conn->name_len = 0;
	memset(pending_conn->name, '\0', USERNAME_MAX_LENGTH + 1);
	pending_conn->affiliated = NULL;
	pending_conn->deferred_disconnect = 0;

	struct sockaddr_in cad;
	int alen = sizeof(cad);
	pending_conn->sd = ACCEPT(l_sd, (struct sockaddr *)&cad, &alen);
	if (pending_conn->sd == INVALID) {
		fprintf(stderr, "Error: socket accept failed\n");
		return FAILURE;
	}

	return SUCCESS;
}


int validate_username(char *name, int name_len, int type, ServerState *state) {
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

	/* Check username availability */
	for (int i = 0; i < state->p_count; i++) {
		if (strcmp(state->p_conns[i]->name, name) == 0) {
			if (type == PENDING_OBSERVER) {
				return state->p_conns[i]->affiliated == NULL ? 'Y' : 'T';
			} else {
				return 'T';
			}
		}
	}

	return type == PENDING_PARTICIPANT ? 'Y' : 'N';
}

int enqueue_message(char *msg, ServerState *state) {
	for (int i = 0; i < state->o_count; i++) {
		Connection *conn = state->o_conns[i];
		if (conn->queue_len < QUEUE_MAX) {
			fprintf(stdout, "Enqueueing msg for Observer (%s|%d): %s\n", conn->name, conn->sd, msg);
			conn->msg_queue[conn->queue_len++] = msg;
		} else {
			fprintf(stderr, "Error: Observer (%s|%d) message queue is too full; not appending msg\n", conn->name, conn->sd);
		}
	}

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

		/* Check participant connections */
		for (int i = 0; i < state.p_count; i++) {
			Connection *conn = state.p_conns[i];

			if (FD_ISSET(conn->sd, &state.read_set)) {
				int status;
				uint16_t msg_len;
				uint16_t net_order;

				fprintf(stdout, "Recving msg len... ");
				status = recv_(conn, &net_order, sizeof(net_order), NO_FLAGS, &state);
				msg_len = ntohs(net_order);
				fprintf(stdout, "(%d -> %d)\n", net_order, msg_len);

				if (status == SUCCESS) {
					char msg[msg_len];

					fprintf(stdout, "Recving msg... ");
					status = recv_(conn, &msg, sizeof(msg), NO_FLAGS, &state);
					fprintf(stdout, "%s\n", msg);

					int len;
					while ((len = strlen(msg)) > msg_len) {
						msg[len-1] = '\0';
					}

					if (status == SUCCESS) {
						char broadcast[14 + msg_len];
						sprintf(broadcast, "> %s: %s", conn->name, msg);
						enqueue_message(broadcast, &state);
						fprintf(stdout, "Broadcasting: %s\n", broadcast);
					}
				}

				if (status == INVALID) {
					if (remove_connection(conn, &state) != SUCCESS) {
						fprintf(stderr, "Error: unable to remove connection (%s|%d).\n", conn->name, conn->sd);
					} else if (conn->type == PARTICIPANT) {
						char msg[14 + conn->name_len];
						sprintf(msg, "User %s has left", conn->name);
						enqueue_message(msg, &state);

						if (conn->affiliated != NULL) {
							conn->affiliated->deferred_disconnect = 1;
						}
						i--;
					}
				}
			}
		}

		/* Check observer connections */
		for (int i = 0; i < state.o_count; i++) {
			Connection *conn = state.o_conns[i];

			if (FD_ISSET(conn->sd, &state.write_set) && conn->queue_len > 0) {
				/* Pop message from front of observer's queue */
				char *msg = conn->msg_queue[0];
				for (int j = 0; j < conn->queue_len - 1; j++) {
					conn->msg_queue[j] = conn->msg_queue[j+1];
				}
				conn->queue_len--;
				fprintf(stdout, "Popped first message from Observer (%s|%d) msg queue: %s\n", conn->name, conn->sd, msg);

				/* Send message to observer */
				uint16_t msg_len = strlen(msg);
				uint16_t net_order = htons(msg_len);

				fprintf(stdout, "Sending message len (%d -> %d)\n", msg_len, net_order);
				if (send(conn->sd, &net_order, sizeof(net_order), NO_FLAGS) < SUCCESS) {
					fprintf(stderr, "Error: unable to send message len (socket error).\n");

				} else {
					fprintf(stdout, "Sending message: %s\n", msg);
					if (send(conn->sd, msg, sizeof(char) * msg_len, NO_FLAGS) < SUCCESS) {
						fprintf(stderr, "Error: unable to send message (socket error).\n");
					}
				}
			} else if (conn->deferred_disconnect && conn->queue_len == 0) {
				if (remove_connection(conn, &state) != SUCCESS) {
					fprintf(stderr, "Error: unable to remove connection (%s|%d).\n", conn->name, conn->sd);
				} else {
					i--;
				}
			}
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
				if (recv_(pending_conn, &net_order, sizeof(net_order), NO_FLAGS, &state) < SUCCESS) {
					fprintf(stderr, "Error: unable to recv username len from participant (socket error).\n");
				}
				name_len = ntohs(net_order);
				fprintf(stdout, "%d -> %d\n", net_order, name_len);

				fprintf(stdout, "Recving username (%d)... ", pending_conn->sd);
				char name[name_len + 1];
				if (recv_(pending_conn, name, sizeof(name), NO_FLAGS, &state) < SUCCESS) {
					fprintf(stderr, "Error: unable to recv username from participant (socket error).\n");
				}
				name[name_len] = '\0';
				fprintf(stdout, "%s\n", name);

				/* Send username confirmation */
				response = validate_username(name, name_len, pending_conn->type, &state);
				fprintf(stdout, "Sending username confirmation (%c)\n", response);
				if (send(pending_conn->sd, &response, sizeof(char), NO_FLAGS) < SUCCESS) {
					fprintf(stderr, "Error: unable to send username confirmation to participant (socket error).\n");
				}

				pending_conn->name_len = name_len;
				strcpy(pending_conn->name, name);
				if (pending_conn->type == PENDING_PARTICIPANT && response == 'Y') {

					char msg[16 + pending_conn->name_len];
					sprintf(msg, "User %s has joined", pending_conn->name);
					enqueue_message(msg, &state);

					/* Add pending_conn to active participants */
					fprintf(stdout, "Adding active participant (%s|%d) to p_conns\n", pending_conn->name, pending_conn->sd);
					Connection *active_conn = state.p_conns[state.p_count++];
					active_conn->sd = pending_conn->sd;
					active_conn->type = PARTICIPANT;
					active_conn->name_len = pending_conn->name_len;
					strcpy(active_conn->name, pending_conn->name);
					active_conn->affiliated = NULL;
					active_conn->deferred_disconnect = 0;

				} else if (pending_conn->type == PENDING_OBSERVER && response == 'Y') {
					fprintf(stdout, "Affiliating observer (%s|%d) to active participant\n", pending_conn->name, pending_conn->sd);

					/* Affiliate pending_conn with corresponding active participant */
					Connection *active_conn = state.o_conns[state.o_count++];
					active_conn->sd = pending_conn->sd;
					active_conn->type = OBSERVER;
					active_conn->name_len = pending_conn->name_len;
					strcpy(active_conn->name, pending_conn->name);
					active_conn->deferred_disconnect = 0;

					active_conn->queue_len = 0;
					active_conn->msg_queue = (char **) malloc(sizeof(char) * MSG_MAX_LEN * QUEUE_MAX);
					for (int i = 0; i < QUEUE_MAX; i++) {
						active_conn->msg_queue[i] = malloc(sizeof(char) * MSG_MAX_LEN);
					}

					for (int k = 0; k < state.p_count; k++) {
						if (strcmp(state.p_conns[k]->name, name) == 0) {
							state.p_conns[k]->affiliated = active_conn;
							active_conn->affiliated = state.p_conns[k];
							break;
						}
					}

				} else if (response == 'N') {
					pending_conn->name_len = name_len;
					strcpy(pending_conn->name, name);
					close(pending_conn->sd);
				}

				if (response != 'I' && response != 'T') {
					/* Remove connection from pending_conns */
					fprintf(stdout, "Removing pending connection (%s|%d)\n", pending_conn->name, pending_conn->sd);
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

		/* Reset fd_sets */
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
