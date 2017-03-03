#include "prog3_server.h"
#include "unity.h"




















void test_server_should_fail_with_invalid_argument_count(void) {

 int argc, ret_code;

 char *argv[3];

 argv[0] = "./prog3_server";

 argv[1] = "36701";

 argv[2] = "36702";



 argc = 2;

 ret_code = main_server(argc, argv);

 UnityAssertEqualNumber((UNITY_INT)((

1

)), (UNITY_INT)((ret_code)), (

((void *)0)

), (UNITY_UINT)(25), UNITY_DISPLAY_STYLE_INT);



 argc = 4;

 ret_code = main_server(argc, argv);

 UnityAssertEqualNumber((UNITY_INT)((

1

)), (UNITY_INT)((ret_code)), (

((void *)0)

), (UNITY_UINT)(29), UNITY_DISPLAY_STYLE_INT);

}



void test_init_listener_should_return_sd_when_port_valid(void) {

 int port = 36701, ret_val;



 ret_val = init_listener(port);

 if (((-1) != (ret_val))) {} else {UnityFail( ((" Expected Not-Equal")), (UNITY_UINT)((UNITY_UINT)(36)));};

}



void test_init_listener_should_return_INVALID_when_port_invalid_or_socket_creation_fails(void) {

 int port1 = -22, port2 = 400, ret_val;



 ret_val = init_listener(port1);

 UnityAssertEqualNumber((UNITY_INT)((-1)), (UNITY_INT)((ret_val)), (

((void *)0)

), (UNITY_UINT)(43), UNITY_DISPLAY_STYLE_INT);



 ret_val = init_listener(port2);

 UnityAssertEqualNumber((UNITY_INT)((-1)), (UNITY_INT)((ret_val)), (

((void *)0)

), (UNITY_UINT)(46), UNITY_DISPLAY_STYLE_INT);

}



void test_handle_listener_should_do_nothing_if_new_connection_fails(void) {

 ServerState state;

 int dummy_listener = 5;

 char ret_val;



 init_server_state(&state);



 ret_val = handle_listener(dummy_listener, &state);

 UnityAssertEqualNumber((UNITY_INT)(UNITY_UINT8 )(('!')), (UNITY_INT)(UNITY_UINT8 )((ret_val)), (

((void *)0)

), (UNITY_UINT)(57), UNITY_DISPLAY_STYLE_UINT8);

}



void test_handle_listener_should_decline_pending_participant_when_at_limit(void) {

 mock_accept = mock_accept_success;



 ServerState state;

 char ret_val;



 init_server_state(&state);

 state.p_count = 255;

 state.p_listener = init_listener(36701);



 ret_val = handle_listener(state.p_listener, &state);

 UnityAssertEqualNumber((UNITY_INT)(UNITY_UINT8 )(('N')), (UNITY_INT)(UNITY_UINT8 )((ret_val)), (

((void *)0)

), (UNITY_UINT)(71), UNITY_DISPLAY_STYLE_UINT8);



 UnityAssertEqualNumber((UNITY_INT)((255)), (UNITY_INT)((state.p_count)), (

((void *)0)

), (UNITY_UINT)(73), UNITY_DISPLAY_STYLE_INT);



 mock_accept = accept;

}



void test_handle_listener_should_accept_pending_participant_when_below_limit(void) {

 mock_accept = mock_accept_success;



 ServerState state;

 char ret_val;



 init_server_state(&state);

 state.p_count = 255 - 20;

 state.o_count = state.p_count;

 state.p_listener = init_listener(36701);



 ret_val = handle_listener(state.p_listener, &state);

 UnityAssertEqualNumber((UNITY_INT)(UNITY_UINT8 )(('Y')), (UNITY_INT)(UNITY_UINT8 )((ret_val)), (

((void *)0)

), (UNITY_UINT)(90), UNITY_DISPLAY_STYLE_UINT8);



 mock_accept = accept;

}



void test_handle_listener_should_decline_pending_observer_when_at_limit(void) {

 mock_accept = mock_accept_success;



 ServerState state;

 char ret_val;



 init_server_state(&state);

 state.o_count = 255;

 state.p_listener = init_listener(36701);



 ret_val = handle_listener(state.p_listener, &state);

 UnityAssertEqualNumber((UNITY_INT)(UNITY_UINT8 )(('N')), (UNITY_INT)(UNITY_UINT8 )((ret_val)), (

((void *)0)

), (UNITY_UINT)(106), UNITY_DISPLAY_STYLE_UINT8);



 mock_accept = accept;

}



void test_handle_listener_should_accept_pending_observer_when_below_limit(void) {

 mock_accept = mock_accept_success;



 ServerState state;

 char ret_val;



 init_server_state(&state);

 state.o_count = 255 - 20;

 state.p_count = state.o_count;

 state.p_listener = init_listener(36701);



 ret_val = handle_listener(state.p_listener, &state);

 UnityAssertEqualNumber((UNITY_INT)(UNITY_UINT8 )(('Y')), (UNITY_INT)(UNITY_UINT8 )((ret_val)), (

((void *)0)

), (UNITY_UINT)(123), UNITY_DISPLAY_STYLE_UINT8);



 mock_accept = accept;

}



void test_new_connection_should_initialize_new_participant_connection_correctly(void) {

 mock_accept = mock_accept_success;



 ServerState state;

 int listener = init_listener(36701), ret_val;



 init_server_state(&state);

 state.p_listener = listener;



 ret_val = new_connection(listener, &state);

 UnityAssertEqualNumber((UNITY_INT)((1)), (UNITY_INT)((ret_val)), (("Socket failure.")), (UNITY_UINT)(138), UNITY_DISPLAY_STYLE_INT);



 UnityAssertEqualNumber((UNITY_INT)((1)), (UNITY_INT)((state.pending_conn->type)), (

((void *)0)

), (UNITY_UINT)(140), UNITY_DISPLAY_STYLE_INT);

 if (((-1) != (state.pending_conn->sd))) {} else {UnityFail( ((" Expected Not-Equal")), (UNITY_UINT)((UNITY_UINT)(141)));};

 UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((state.pending_conn->name_len)), (

((void *)0)

), (UNITY_UINT)(142), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualString((const char*)(("")), (const char*)((state.pending_conn->name)), (

((void *)0)

), (UNITY_UINT)(143));



 mock_accept = accept;

}



void test_new_connection_should_initialize_new_observer_connection_correctly(void) {

 mock_accept = mock_accept_success;



 ServerState state;

 int listener = init_listener(36701), ret_val;



 init_server_state(&state);

 state.o_listener = listener;



 ret_val = new_connection(listener, &state);

 UnityAssertEqualNumber((UNITY_INT)((1)), (UNITY_INT)((ret_val)), (("Socket failure.")), (UNITY_UINT)(158), UNITY_DISPLAY_STYLE_INT);



 UnityAssertEqualNumber((UNITY_INT)((2)), (UNITY_INT)((state.pending_conn->type)), (

((void *)0)

), (UNITY_UINT)(160), UNITY_DISPLAY_STYLE_INT);

 if (((-1) != (state.pending_conn->sd))) {} else {UnityFail( ((" Expected Not-Equal")), (UNITY_UINT)((UNITY_UINT)(161)));};

 UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((state.pending_conn->name_len)), (

((void *)0)

), (UNITY_UINT)(162), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualString((const char*)(("")), (const char*)((state.pending_conn->name)), (

((void *)0)

), (UNITY_UINT)(163));



 mock_accept = accept;

}
