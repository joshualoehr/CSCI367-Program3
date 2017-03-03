#include "prog3_participant.h"
#include "prog3_server.h"
#include "unity.h"






















void init_participant_state(ParticipantState *state) {

 state->sd = 1000;

 strcpy(state->name, "validname");

 state->name_len = strlen(state->name);

}







void test_participant_should_fail_with_invalid_arguments(void) {

 int argc, ret_code;

 char *argv[3];

 argv[0] = "./prog3_participant";

 argv[1] = "localhost";

 argv[2] = "36701";



 argc = 2;

 ret_code = main_participant(argc, argv);

 UnityAssertEqualNumber((UNITY_INT)((

1

)), (UNITY_INT)((ret_code)), (

((void *)0)

), (UNITY_UINT)(35), UNITY_DISPLAY_STYLE_INT);



 argc = 4;

 ret_code = main_participant(argc, argv);

 UnityAssertEqualNumber((UNITY_INT)((

1

)), (UNITY_INT)((ret_code)), (

((void *)0)

), (UNITY_UINT)(39), UNITY_DISPLAY_STYLE_INT);

}



void test_init_connection_should_return_sd_when_port_and_host_valid(void) {

 mock_connect = mock_connect_success;



 char *host = "localhost";

 int port = 36701, ret_val;



 ret_val = init_connection(host, port);

 if (((-1) != (ret_val))) {} else {UnityFail( ((" Expected Not-Equal")), (UNITY_UINT)((UNITY_UINT)(49)));};



 mock_connect = connect;

}



void test_init_connection_should_return_INVALID_when_port_or_host_invalid(void) {

 char *host = "localhost";

 int port1 = -22, port2 = 400, ret_val;



 ret_val = init_connection(host, port1);

 UnityAssertEqualNumber((UNITY_INT)((-1)), (UNITY_INT)((ret_val)), (

((void *)0)

), (UNITY_UINT)(59), UNITY_DISPLAY_STYLE_INT);



 ret_val = init_connection(host, port2);

 UnityAssertEqualNumber((UNITY_INT)((-1)), (UNITY_INT)((ret_val)), (

((void *)0)

), (UNITY_UINT)(62), UNITY_DISPLAY_STYLE_INT);

}



void test_init_connection_should_return_INVALID_when_connect_fails(void) {

 mock_connect = mock_connect_failure;



 char *host = "localhost";

 int port = 36701, ret_val;



 ret_val = init_connection(host, port);

 UnityAssertEqualNumber((UNITY_INT)((-1)), (UNITY_INT)((ret_val)), (

((void *)0)

), (UNITY_UINT)(72), UNITY_DISPLAY_STYLE_INT);



 mock_connect = connect;

}



void test_validate_username_should_return_INVALID_if_username_invalid(void) {

 UnityAssertEqualNumber((UNITY_INT)((-1)), (UNITY_INT)((validate_username(""))), (

((void *)0)

), (UNITY_UINT)(78), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualNumber((UNITY_INT)((-1)), (UNITY_INT)((validate_username("abcdefghijklmnopqrstuvwxyz"))), (

((void *)0)

), (UNITY_UINT)(79), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualNumber((UNITY_INT)((-1)), (UNITY_INT)((validate_username("&*(^%%"))), (

((void *)0)

), (UNITY_UINT)(80), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualNumber((UNITY_INT)((-1)), (UNITY_INT)((validate_username("my name"))), (

((void *)0)

), (UNITY_UINT)(81), UNITY_DISPLAY_STYLE_INT);

}



void test_validate_username_should_return_SUCCESS_if_username_valid(void) {

 UnityAssertEqualNumber((UNITY_INT)((1)), (UNITY_INT)((validate_username("username"))), (

((void *)0)

), (UNITY_UINT)(85), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualNumber((UNITY_INT)((1)), (UNITY_INT)((validate_username("USERNAME"))), (

((void *)0)

), (UNITY_UINT)(86), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualNumber((UNITY_INT)((1)), (UNITY_INT)((validate_username("user_name"))), (

((void *)0)

), (UNITY_UINT)(87), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualNumber((UNITY_INT)((1)), (UNITY_INT)((validate_username("user123"))), (

((void *)0)

), (UNITY_UINT)(88), UNITY_DISPLAY_STYLE_INT);

}



void test_negotiate_username_should_send_username_length_followed_by_username_to_server(void) {



}



void test_negotiate_username_should_return_FAILURE_if_timer_expires(void) {

 mock_prompt_and_get_username = mock_prompt_and_get_username_valid;

 mock_send_username = mock_send_username_success;

 mock_recv_negotiation = mock_recv_negotiation_timeout;



 ParticipantState state;

 init_participant_state(&state);



 UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((negotiate_username(&state))), (

((void *)0)

), (UNITY_UINT)(103), UNITY_DISPLAY_STYLE_INT);



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



 UnityAssertEqualNumber((UNITY_INT)((-1)), (UNITY_INT)((negotiate_username(&state))), (

((void *)0)

), (UNITY_UINT)(121), UNITY_DISPLAY_STYLE_INT);



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



 UnityAssertEqualNumber((UNITY_INT)((-1)), (UNITY_INT)((negotiate_username(&state))), (

((void *)0)

), (UNITY_UINT)(139), UNITY_DISPLAY_STYLE_INT);



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



 UnityAssertEqualNumber((UNITY_INT)((1)), (UNITY_INT)((negotiate_username(&state))), (

((void *)0)

), (UNITY_UINT)(157), UNITY_DISPLAY_STYLE_INT);



#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

 mock_prompt_and_get_username = prompt_and_get_username;

 mock_send_username = send;

 mock_recv_negotiation = recv;

#pragma GCC diagnostic pop

}
