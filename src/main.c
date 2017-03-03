
#define ARGC 3

#include "prog3_server.h"
#include "prog3_participant.h"
#include "prog3_observer.h"

#include <stdio.h>

void init_argv(char **server_argv, char **participant_argv, char **observer_argv) {
	server_argv[0] = "./prog3_server";
	server_argv[1] = "36701";
	server_argv[2] = "36702";

//	participant_argv[0] = "./prog3_participant";
//	participant_argv[1] = "localhost";
//	participant_argv[2] = "36701";
//
//	observer_argv[0] = "./prog3_observer";
//	observer_argv[1] = "localhost";
//	observer_argv[2] = "36702";
}
