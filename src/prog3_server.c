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
#include <time.h>
#include <unistd.h>

int (*mock_accept)(int sockfd, struct sockaddr *addr, socklen_t *addrlen) = accept;

/* HELPER FUNCTIONS */

int recv_(Connection *conn, void *buf, size_t len, int flags, ServerState *state) {
	int status = recv(conn->sd, buf, len, flags);
	if (status == 0) {
		conn->disconnect = 1;
		return INVALID;
	} else if (status < 0) {
		fprintf(stderr, "Error: unable to recv message (socket error)\n");
		return FAILURE;
	}

	return SUCCESS;
}

//time_t find_smallest_timeout(time_t now, ServerState *state) {
//	time_t min = TIMEOUT;
//	for (int i = 0; i < state->pending_count; i++) {
//		time_t diff = state->pending_conns[i]->timeout - now;
//		min = (diff < min) ? diff : min;
//	}
//	return min;
//}

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

	state->timeout.tv_sec = 1;
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
	pending_conn->disconnect = 0;
	pending_conn->deferred_disconnect = 0;
	pending_conn->timeout.tv_sec = TIMEOUT;
	pending_conn->timeout.tv_usec = 0;

	struct sockaddr_in cad;
	int alen = sizeof(cad);
	pending_conn->sd = ACCEPT(l_sd, (struct sockaddr *)&cad, &alen);
	if (pending_conn->sd == INVALID) {
		fprintf(stderr, "Error: socket accept failed\n");
		return FAILURE;
	}

	return SUCCESS;
}


int remove_connection(Connection *conn, ServerState *state) {
	/* Remove connection from pending_conns */
	fprintf(stdout, "Removing connection (%s|%d)\n",
			conn->type == PENDING_PARTICIPANT || conn->type == PENDING_OBSERVER ? "PENDING" : conn->name, conn->sd);

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
	*conn_count = *conn_count - 1;

	if (conn->type == OBSERVER) {
		conn->affiliated->affiliated = NULL;
	}

	if (conn->type != PENDING_PARTICIPANT && conn->type != PENDING_OBSERVER) {
		close(conn->sd);
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

int dequeue_and_send_msg(Connection *conn) {
	uint16_t msg_len = conn->msg_queue_lens[0];
	char msg[msg_len];
	strcpy(msg, conn->msg_queue[0]);

	for (int j = 0; j < conn->queue_len - 1; j++) {
		conn->msg_queue_lens[j] = conn->msg_queue_lens[j+1];
		conn->msg_queue[j] = conn->msg_queue[j+1];
	}
	conn->queue_len--;
	fprintf(stdout, "Popped msg from obsv queue (%s|%d): (len: %d) - %s\n", conn->name, conn->sd, msg_len, msg);

	/* Send message to observer */
	uint16_t net_order = htons(msg_len);
	fprintf(stdout, "Sending message len (%d -> %d)\n", msg_len, net_order);
	if (send(conn->sd, &net_order, sizeof(net_order), NO_FLAGS) < SUCCESS) {
		fprintf(stderr, "Error: unable to send message len (socket error).\n");
		return FAILURE;

	}

	fprintf(stdout, "Sending message: %s\n", msg);
	if (send(conn->sd, msg, sizeof(char) * msg_len, NO_FLAGS) < SUCCESS) {
		fprintf(stderr, "Error: unable to send message (socket error).\n");
		return FAILURE;
	}

	return SUCCESS;
}

int enqueue_msg(uint16_t msg_len, char *msg, char *recipient, ServerState *state) {
	int multicast = strcmp(MULTICAST, recipient) == 0;

	for (int i = 0; i < state->p_count; i++) {
		Connection *conn = state->p_conns[i];

		if (multicast || strcmp(recipient, conn->name) == 0) {
			Connection *obs = conn->affiliated;

			/* If this participant has no observer, there is nothing to do */
			if (obs == NULL) return SUCCESS;

			if (obs->queue_len < QUEUE_MAX) {
				fprintf(stdout, "Obs(%s|%d) Enqueueing msg (len %d): %s\n", obs->name, obs->sd, msg_len, msg);
				obs->msg_queue_lens[obs->queue_len] = msg_len;
				strcpy(obs->msg_queue[obs->queue_len], msg);
				obs->queue_len++;

				if (!multicast) return SUCCESS;
			} else {
				fprintf(stderr, "Error: Obs(%s|%d) message queue is full\n", obs->name, obs->sd);
				if (!multicast) return FAILURE;
			}
		}
	}

	/* If a unicast message gets here, the recipient does not exist */
	return multicast ? SUCCESS : INVALID;
}

int broadcast_user_msg(uint16_t msg_len, char *msg, char *src_name, ServerState *state) {
	uint16_t broadcast_len = msg_len + 14;
	char broadcast[broadcast_len];
	char dst_name[INC_MSG_MAX_LEN];

	int private = sscanf(msg, "@%s", dst_name);
	if (private && msg[1] == ' ') private = 0; // deal with strange edge case
	sprintf(broadcast, "%c %10s: %s", private ? '*' : '>', src_name, msg);

	int status;
	if (private) {
		if ((status = enqueue_msg(broadcast_len, broadcast, dst_name, state)) == SUCCESS) {
			if (strcmp(src_name, dst_name) != 0) {
				enqueue_msg(broadcast_len, broadcast, src_name, state);
			}

		} else if (status == INVALID) {
			uint16_t warning_len = 31 + strlen(dst_name);
			char warning[warning_len];
			sprintf(warning, "Warning: user %s doesn't exist...", dst_name);
			enqueue_msg(warning_len, warning, src_name, state);

		} else {
			return FAILURE;
		}
	} else {
		if (enqueue_msg(broadcast_len, broadcast, MULTICAST, state) == FAILURE) {
			return FAILURE;
		}
	}

	return SUCCESS;
}

int broadcast_server_msg(char *msg, int name_len, char *name, ServerState *state) {
	if (name_len) {
		name[name_len] = '\0';
		uint16_t broadcast_len = strlen(msg) - 2 + name_len;
		char broadcast[broadcast_len];
		sprintf(broadcast, msg, name);
		return enqueue_msg(broadcast_len, broadcast, MULTICAST, state);
	} else {
		return enqueue_msg(strlen(msg), msg, MULTICAST, state);
	}
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

	time_t now;
	while (1) {
		time(&now);
		struct timeval timeout;
		timeout.tv_sec = state.timeout.tv_sec;
		timeout.tv_usec = state.timeout.tv_usec;

		/* Select sockets and update fd_sets */
		state.read_set = state.master_set;
		state.write_set = state.master_set;
		if (select(state.fd_max + 1, &state.read_set, &state.write_set, NULL, &timeout) == INVALID) {
			fprintf(stderr, "Error: unable to select.\n");
			return EXIT_FAILURE;
		}

		struct timeval timediff;
		timediff.tv_sec = state.timeout.tv_sec - timeout.tv_sec;
		timediff.tv_usec = state.timeout.tv_usec - timeout.tv_usec;

		//fprintf(stdout, "timediff: %ds %dus\n", timediff.tv_sec, timediff.tv_usec);

		/* Check participant connections */
		for (int i = 0; i < state.p_count; i++) {
			Connection *conn = state.p_conns[i];

			if (FD_ISSET(conn->sd, &state.read_set)) {
				int status;
				uint16_t msg_len;
				uint16_t net_order;

				status = recv_(conn, &net_order, sizeof(net_order), NO_FLAGS, &state);
				msg_len = ntohs(net_order);
				fprintf(stdout, "Recv'd msg len (%d -> %d)\n", net_order, msg_len);

				if (status != SUCCESS || msg_len > INC_MSG_MAX_LEN) {
					conn->disconnect = 1;
				} else {
					char msg[msg_len];
					if (recv_(conn, &msg, sizeof(char) * msg_len, NO_FLAGS, &state) == SUCCESS) {
						broadcast_user_msg(msg_len, msg, conn->name, &state);
						fprintf(stdout, "Recv'd msg %s\n", msg);
					}
				}

				if (conn->disconnect) {
					broadcast_server_msg("User %s has left", conn->name_len, conn->name, &state);
					if (remove_connection(conn, &state) == SUCCESS) {
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
				dequeue_and_send_msg(conn);

			} else if (FD_ISSET(conn->sd, &state.read_set) || (conn->deferred_disconnect && conn->queue_len == 0)) {
				if (remove_connection(conn, &state) == SUCCESS) {
					i--;
				}
			}
		}

		/* Check listeners */
		if (FD_ISSET(state.p_listener, &state.read_set)) {
			negotiate_connection(state.p_listener, &state);
		}
		if (FD_ISSET(state.o_listener, &state.read_set)) {
			negotiate_connection(state.o_listener, &state);
		}

		/* Check pending connections */
		for (int i = 0; i < state.pending_count; i++) {
			Connection *pending_conn = state.pending_conns[i];

			int orig_sec = pending_conn->timeout.tv_sec;

			/* Update timeout values */
			pending_conn->timeout.tv_sec -= timediff.tv_sec;
			pending_conn->timeout.tv_usec -= timediff.tv_usec;
			while (pending_conn->timeout.tv_usec < 0) {
				pending_conn->timeout.tv_sec -= 1;
				pending_conn->timeout.tv_usec += 1000000;
			}

			if (pending_conn->timeout.tv_sec != orig_sec)
				fprintf(stdout, "(PENDING|%d|%d;%d)\n", pending_conn->sd, pending_conn->timeout.tv_sec, pending_conn->timeout.tv_usec);

			/* Disconnect if timeout expires */
			if ( pending_conn->timeout.tv_sec < 0 ||
				(pending_conn->timeout.tv_sec == 0 && pending_conn->timeout.tv_usec == 0) ) {
				close(pending_conn->sd);
				remove_connection(pending_conn, &state);
				i--;

			} else if (FD_ISSET(pending_conn->sd, &state.read_set)) {
				char response = 0;

				/* Receive username (length, then string) */
				uint8_t name_len = 0;
				if (recv_(pending_conn, &name_len, sizeof(name_len), NO_FLAGS, &state) == SUCCESS) {
					fprintf(stdout, "(PENDING|%d) Recv'd username len (%d)\n", pending_conn->sd, name_len);
					pending_conn->name_len = name_len;

					char name[name_len];
					if (recv_(pending_conn, name, sizeof(char) * name_len, NO_FLAGS, &state) == SUCCESS) {
						fprintf(stdout, "(PENDING|%d) Recv'd username (%s)\n", pending_conn->sd, name);
						strcpy(pending_conn->name, name);
						for (int j = name_len; j < USERNAME_MAX_LENGTH; j++) {
							pending_conn->name[j] = '\0';
						}

						response = validate_username(pending_conn->name, name_len, pending_conn->type, &state);
						if (send(pending_conn->sd, &response, sizeof(char), NO_FLAGS) < SUCCESS) {
							fprintf(stderr, "Error: unable to send username confirmation to participant (socket error).\n");
						}
					}
				}

				if (response == 'Y') {

					if (pending_conn->type == PENDING_PARTICIPANT) {
						/* Add pending_conn to active participants */
						Connection *active_conn = state.p_conns[state.p_count++];
						active_conn->sd = pending_conn->sd;
						active_conn->type = PARTICIPANT;
						active_conn->name_len = pending_conn->name_len;

						strcpy(active_conn->name, pending_conn->name);
						active_conn->affiliated = NULL;
						active_conn->disconnect = 0;
						active_conn->deferred_disconnect = 0;

						broadcast_server_msg("User %s has joined", active_conn->name_len, active_conn->name, &state);
						pending_conn->disconnect = 1;

					} else if (pending_conn->type == PENDING_OBSERVER) {

						broadcast_server_msg("A new observer has joined", 0, NULL, &state);

						/* Affiliate pending_conn with corresponding active participant */
						Connection *active_conn = state.o_conns[state.o_count++];
						active_conn->sd = pending_conn->sd;
						active_conn->type = OBSERVER;
						active_conn->name_len = pending_conn->name_len;
						strcpy(active_conn->name, pending_conn->name);
						active_conn->disconnect = 0;
						active_conn->deferred_disconnect = 0;

						active_conn->queue_len = 0;
						active_conn->msg_queue_lens = (uint16_t *) malloc(sizeof(uint16_t) * QUEUE_MAX);
						active_conn->msg_queue = (char **) malloc(sizeof(char) * OUT_MSG_MAX_LEN * QUEUE_MAX);
						for (int j = 0; j < QUEUE_MAX; j++) {
							active_conn->msg_queue[j] = malloc(sizeof(char) * OUT_MSG_MAX_LEN);
						}

						for (int k = 0; k < state.p_count; k++) {
							if (strcmp(state.p_conns[k]->name, active_conn->name) == 0) {
								state.p_conns[k]->affiliated = active_conn;
								active_conn->affiliated = state.p_conns[k];
								break;
							}
						}

						pending_conn->disconnect = 1;
					}
				} else if (response == 'N') {
					pending_conn->disconnect = 1;
					close(pending_conn->sd);
				} else if (response == 'T') {
					pending_conn->timeout.tv_sec = TIMEOUT;
					pending_conn->timeout.tv_usec = 0;
				}

				if (pending_conn->disconnect) {
					remove_connection(pending_conn, &state);
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

	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	return main_server(argc, argv);
}


/* MOCKED FUNCTIONS */

int mock_accept_success(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	return 1000;
}
