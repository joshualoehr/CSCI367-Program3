/* AUTOGENERATED FILE. DO NOT EDIT. */
#ifndef _MOCK_PROG3_PARTICIPANT_H
#define _MOCK_PROG3_PARTICIPANT_H

#include "prog3_participant.h"

/* Ignore the following warnings, since we are copying code */
#if defined(__GNUC__) && !defined(__ICC) && !defined(__TMS470__)
#if !defined(__clang__)
#pragma GCC diagnostic ignored "-Wpragmas"
#endif
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wduplicate-decl-specifier"
#endif

void mock_prog3_participant_Init(void);
void mock_prog3_participant_Destroy(void);
void mock_prog3_participant_Verify(void);




#define main_participant_IgnoreAndReturn(cmock_retval) main_participant_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void main_participant_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define main_participant_ExpectAndReturn(argc, argv, cmock_retval) main_participant_CMockExpectAndReturn(__LINE__, argc, argv, cmock_retval)
void main_participant_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, int argc, char** argv, int cmock_to_return);
typedef int (* CMOCK_main_participant_CALLBACK)(int argc, char** argv, int cmock_num_calls);
void main_participant_StubWithCallback(CMOCK_main_participant_CALLBACK Callback);

#endif
