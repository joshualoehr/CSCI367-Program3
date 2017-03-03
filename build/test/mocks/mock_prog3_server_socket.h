/* AUTOGENERATED FILE. DO NOT EDIT. */
#ifndef _MOCK_PROG3_SERVER_SOCKET_H
#define _MOCK_PROG3_SERVER_SOCKET_H

#include "prog3_server_socket.h"

/* Ignore the following warnings, since we are copying code */
#if defined(__GNUC__) && !defined(__ICC) && !defined(__TMS470__)
#if !defined(__clang__)
#pragma GCC diagnostic ignored "-Wpragmas"
#endif
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wduplicate-decl-specifier"
#endif

void mock_prog3_server_socket_Init(void);
void mock_prog3_server_socket_Destroy(void);
void mock_prog3_server_socket_Verify(void);




#define accept_client_connection_IgnoreAndReturn(cmock_retval) accept_client_connection_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void accept_client_connection_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define accept_client_connection_ExpectAndReturn(l_sd, cmock_retval) accept_client_connection_CMockExpectAndReturn(__LINE__, l_sd, cmock_retval)
void accept_client_connection_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, int l_sd, int cmock_to_return);
typedef int (* CMOCK_accept_client_connection_CALLBACK)(int l_sd, int cmock_num_calls);
void accept_client_connection_StubWithCallback(CMOCK_accept_client_connection_CALLBACK Callback);

#endif
