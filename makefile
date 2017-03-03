CFLAGS = -Wall -g

all: main.c
#all: prog3_server.c prog3_participant.c prog3_observer.c

main.c: src/main.c
	gcc $(CFLAGS) -o build/main src/main.c src/prog3_server.c src/prog3_participant.c src/prog3_observer.c
	
#prog3_server.c: src/prog3_server.c
#	gcc $(CFLAGS) -o build/prog3_server src/prog3_server.c 
#	
#prog3_participant.c: src/prog3_participant.c
#	gcc $(CFLAGS) -o build/prog3_participant src/prog3_participant.c
#
#prog3_observer.c: src/prog3_observer.c
#	gcc $(CFLAGS) -o build/prog3_observer src/prog3_observer.c