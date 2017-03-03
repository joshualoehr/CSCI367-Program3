#include "prog3_observer.h"

#include <stdio.h>
#include <stdlib.h>

int main_observer(int argc, char **argv) {
	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"prog3_observer server_address server_port\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
