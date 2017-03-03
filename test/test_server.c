/*
 * test_server.c
 *
 *  Created on: Feb 23, 2017
 *      Author: loehrj
 */

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unity.h>

#include "prog3_server.h"


void test_server_should_fail_with_invalid_argument_count(void) {
	int argc, ret_code;
	char *argv[3];
	argv[0] = "./prog3_server";
	argv[1] = "36701";
	argv[2] = "36702";

	argc = 2;
	ret_code = main_server(argc, argv);
	TEST_ASSERT_EQUAL_INT(EXIT_FAILURE, ret_code);

	argc = 4;
	ret_code = main_server(argc, argv);
	TEST_ASSERT_EQUAL_INT(EXIT_FAILURE, ret_code);
}

void test_init_listener_should_return_sd_when_port_valid(void) {
	int port = 36701, ret_val;

	ret_val = init_listener(port);
	TEST_ASSERT_NOT_EQUAL(INVALID, ret_val);
}

void test_init_listener_should_return_INVALID_when_port_invalid_or_socket_creation_fails(void) {
	int port1 = -22, port2 = 400, ret_val;

	ret_val = init_listener(port1);
	TEST_ASSERT_EQUAL_INT(INVALID, ret_val);

	ret_val = init_listener(port2);
	TEST_ASSERT_EQUAL_INT(INVALID, ret_val);
}

void test_handle_listener_should_do_nothing_if_new_connection_fails(void) {
	ServerState state;
	int dummy_listener = 5;
	char ret_val;

	init_server_state(&state);

	ret_val = handle_listener(dummy_listener, &state);
	TEST_ASSERT_EQUAL_UINT8('!', ret_val);
}

void test_handle_listener_should_decline_pending_participant_when_at_limit(void) {
	mock_accept = mock_accept_success;

	ServerState state;
	char ret_val;

	init_server_state(&state);
	state.p_count = MAX_PARTICIPANTS;
	state.p_listener = init_listener(36701);

	ret_val = handle_listener(state.p_listener, &state);
	TEST_ASSERT_EQUAL_UINT8('N', ret_val);

	TEST_ASSERT_EQUAL_INT(MAX_PARTICIPANTS, state.p_count);

	mock_accept = accept;
}

void test_handle_listener_should_accept_pending_participant_when_below_limit(void) {
	mock_accept = mock_accept_success;

	ServerState state;
	char ret_val;

	init_server_state(&state);
	state.p_count = MAX_PARTICIPANTS - 20;
	state.o_count = state.p_count;
	state.p_listener = init_listener(36701);

	ret_val = handle_listener(state.p_listener, &state);
	TEST_ASSERT_EQUAL_UINT8('Y', ret_val);

	mock_accept = accept;
}

void test_handle_listener_should_decline_pending_observer_when_at_limit(void) {
	mock_accept = mock_accept_success;

	ServerState state;
	char ret_val;

	init_server_state(&state);
	state.o_count = MAX_OBSERVERS;
	state.p_listener = init_listener(36701);

	ret_val = handle_listener(state.p_listener, &state);
	TEST_ASSERT_EQUAL_UINT8('N', ret_val);

	mock_accept = accept;
}

void test_handle_listener_should_accept_pending_observer_when_below_limit(void) {
	mock_accept = mock_accept_success;

	ServerState state;
	char ret_val;

	init_server_state(&state);
	state.o_count = MAX_OBSERVERS - 20;
	state.p_count = state.o_count;
	state.p_listener = init_listener(36701);

	ret_val = handle_listener(state.p_listener, &state);
	TEST_ASSERT_EQUAL_UINT8('Y', ret_val);

	mock_accept = accept;
}

void test_new_connection_should_initialize_new_participant_connection_correctly(void) {
	mock_accept = mock_accept_success;

	ServerState state;
	int listener = init_listener(36701), ret_val;

	init_server_state(&state);
	state.p_listener = listener;

	ret_val = new_connection(listener, &state);
	TEST_ASSERT_EQUAL_INT_MESSAGE(SUCCESS, ret_val, "Socket failure.");

	TEST_ASSERT_EQUAL_INT(PARTICIPANT, state.pending_conn->type);
	TEST_ASSERT_NOT_EQUAL(INVALID, state.pending_conn->sd);
	TEST_ASSERT_EQUAL_INT(0, state.pending_conn->name_len);
	TEST_ASSERT_EQUAL_STRING("", state.pending_conn->name);

	mock_accept = accept;
}

void test_new_connection_should_initialize_new_observer_connection_correctly(void) {
	mock_accept = mock_accept_success;

	ServerState state;
	int listener = init_listener(36701), ret_val;

	init_server_state(&state);
	state.o_listener = listener;

	ret_val = new_connection(listener, &state);
	TEST_ASSERT_EQUAL_INT_MESSAGE(SUCCESS, ret_val, "Socket failure.");

	TEST_ASSERT_EQUAL_INT(OBSERVER, state.pending_conn->type);
	TEST_ASSERT_NOT_EQUAL(INVALID, state.pending_conn->sd);
	TEST_ASSERT_EQUAL_INT(0, state.pending_conn->name_len);
	TEST_ASSERT_EQUAL_STRING("", state.pending_conn->name);

	mock_accept = accept;
}






