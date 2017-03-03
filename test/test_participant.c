/*
 * test_participant.c
 *
 *  Created on: Feb 23, 2017
 *      Author: loehrj
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

#include "prog3_server.h"
#include "prog3_participant.h"

/* HELPERS */

void init_participant_state(ParticipantState *state) {
	state->sd = 1000;
	strcpy(state->name, "validname");
	state->name_len = strlen(state->name);
}

/* TESTS */

void test_participant_should_fail_with_invalid_arguments(void) {
	int argc, ret_code;
	char *argv[3];
	argv[0] = "./prog3_participant";
	argv[1] = "localhost";
	argv[2] = "36701";

	argc = 2;
	ret_code = main_participant(argc, argv);
	TEST_ASSERT_EQUAL_INT(EXIT_FAILURE, ret_code);

	argc = 4;
	ret_code = main_participant(argc, argv);
	TEST_ASSERT_EQUAL_INT(EXIT_FAILURE, ret_code);
}

void test_init_connection_should_return_sd_when_port_and_host_valid(void) {
	mock_connect = mock_connect_success;

	char *host = "localhost";
	int port = 36701, ret_val;

	ret_val = init_connection(host, port);
	TEST_ASSERT_NOT_EQUAL(INVALID, ret_val);

	mock_connect = connect;
}

void test_init_connection_should_return_INVALID_when_port_or_host_invalid(void) {
	char *host = "localhost";
	int port1 = -22, port2 = 400, ret_val;

	ret_val = init_connection(host, port1);
	TEST_ASSERT_EQUAL_INT(INVALID, ret_val);

	ret_val = init_connection(host, port2);
	TEST_ASSERT_EQUAL_INT(INVALID, ret_val);
}

void test_init_connection_should_return_INVALID_when_connect_fails(void) {
	mock_connect = mock_connect_failure;

	char *host = "localhost";
	int port = 36701, ret_val;

	ret_val = init_connection(host, port);
	TEST_ASSERT_EQUAL(INVALID, ret_val);

	mock_connect = connect;
}

void test_validate_username_should_return_INVALID_if_username_invalid(void) {
	TEST_ASSERT_EQUAL_INT(INVALID, validate_username("")); // Too short
	TEST_ASSERT_EQUAL_INT(INVALID, validate_username("abcdefghijklmnopqrstuvwxyz")); // Too long
	TEST_ASSERT_EQUAL_INT(INVALID, validate_username("&*(^%%")); // Special characters
	TEST_ASSERT_EQUAL_INT(INVALID, validate_username("my name")); // Whitespace
}

void test_validate_username_should_return_SUCCESS_if_username_valid(void) {
	TEST_ASSERT_EQUAL_INT(SUCCESS, validate_username("username")); // Correct length, lowercase allowed
	TEST_ASSERT_EQUAL_INT(SUCCESS, validate_username("USERNAME")); // Uppercase characters
	TEST_ASSERT_EQUAL_INT(SUCCESS, validate_username("user_name")); // Underscores allowed
	TEST_ASSERT_EQUAL_INT(SUCCESS, validate_username("user123")); // numbers allowed
}

void test_negotiate_username_should_send_username_length_followed_by_username_to_server(void) {

}

void test_negotiate_username_should_return_FAILURE_if_timer_expires(void) {
	mock_prompt_and_get_username = mock_prompt_and_get_username_valid;
	mock_send_username = mock_send_username_success;
	mock_recv_negotiation = mock_recv_negotiation_timeout;

	ParticipantState state;
	init_participant_state(&state);

	TEST_ASSERT_EQUAL_INT(FAILURE, negotiate_username(&state));

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
	mock_prompt_and_get_username = prompt_and_get_username;
	mock_send_username = send;
	mock_recv_negotiation = recv;
	#pragma GCC diagnostic pop
}

void test_negotiate_username_should_return_INVALID_if_server_replies_with_I(void) {
	mock_prompt_and_get_username = mock_prompt_and_get_username_valid;
	mock_send_username = mock_send_username_success;
	mock_recv_negotiation = mock_recv_negotiation_I;

	ParticipantState state;
	init_participant_state(&state);

	TEST_ASSERT_EQUAL_INT(INVALID, negotiate_username(&state));

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
	mock_prompt_and_get_username = prompt_and_get_username;
	mock_send_username = send;
	mock_recv_negotiation = recv;
	#pragma GCC diagnostic pop
}

void test_negotiate_username_should_return_INVALID_if_server_replies_with_T(void) {
	mock_prompt_and_get_username = mock_prompt_and_get_username_valid;
	mock_send_username = mock_send_username_success;
	mock_recv_negotiation = mock_recv_negotiation_T;

	ParticipantState state;
	init_participant_state(&state);

	TEST_ASSERT_EQUAL_INT(INVALID, negotiate_username(&state));

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
	mock_prompt_and_get_username = prompt_and_get_username;
	mock_send_username = send;
	mock_recv_negotiation = recv;
	#pragma GCC diagnostic pop
}

void test_negotiate_username_should_return_SUCCESS_if_server_replies_with_Y(void) {
	mock_prompt_and_get_username = mock_prompt_and_get_username_valid;
	mock_send_username = mock_send_username_success;
	mock_recv_negotiation = mock_recv_negotiation_Y;

	ParticipantState state;
	init_participant_state(&state);

	TEST_ASSERT_EQUAL_INT(SUCCESS, negotiate_username(&state));

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
	mock_prompt_and_get_username = prompt_and_get_username;
	mock_send_username = send;
	mock_recv_negotiation = recv;
	#pragma GCC diagnostic pop
}


