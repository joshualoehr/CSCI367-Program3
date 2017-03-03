/* AUTOGENERATED FILE. DO NOT EDIT. */

/*=======Test Runner Used To Run Each Test Below=====*/
#define RUN_TEST(TestFunc, TestLineNum) \
{ \
  Unity.CurrentTestName = #TestFunc; \
  Unity.CurrentTestLineNumber = TestLineNum; \
  Unity.NumberOfTests++; \
  if (TEST_PROTECT()) \
  { \
      setUp(); \
      TestFunc(); \
  } \
  if (TEST_PROTECT() && !TEST_IS_IGNORED) \
  { \
    tearDown(); \
  } \
  UnityConcludeTest(); \
}

/*=======Automagically Detected Files To Include=====*/
#include "unity.h"
#include <setjmp.h>
#include <stdio.h>

int GlobalExpectCount;
int GlobalVerifyOrder;
char* GlobalOrderError;

/*=======External Functions This Runner Calls=====*/
extern void setUp(void);
extern void tearDown(void);
extern void test_participant_should_fail_with_invalid_arguments(void);
extern void test_init_connection_should_return_sd_when_port_and_host_valid(void);
extern void test_init_connection_should_return_INVALID_when_port_or_host_invalid(void);
extern void test_init_connection_should_return_INVALID_when_connect_fails(void);
extern void test_validate_username_should_return_INVALID_if_username_invalid(void);
extern void test_validate_username_should_return_SUCCESS_if_username_valid(void);
extern void test_negotiate_username_should_send_username_length_followed_by_username_to_server(void);
extern void test_negotiate_username_should_return_FAILURE_if_timer_expires(void);
extern void test_negotiate_username_should_return_INVALID_if_server_replies_with_I(void);
extern void test_negotiate_username_should_return_INVALID_if_server_replies_with_T(void);
extern void test_negotiate_username_should_return_SUCCESS_if_server_replies_with_Y(void);


/*=======Test Reset Option=====*/
void resetTest(void);
void resetTest(void)
{
  tearDown();
  setUp();
}


/*=======MAIN=====*/
int main(void)
{
  UnityBegin("test_participant.c");
  RUN_TEST(test_participant_should_fail_with_invalid_arguments, 26);
  RUN_TEST(test_init_connection_should_return_sd_when_port_and_host_valid, 42);
  RUN_TEST(test_init_connection_should_return_INVALID_when_port_or_host_invalid, 54);
  RUN_TEST(test_init_connection_should_return_INVALID_when_connect_fails, 65);
  RUN_TEST(test_validate_username_should_return_INVALID_if_username_invalid, 77);
  RUN_TEST(test_validate_username_should_return_SUCCESS_if_username_valid, 84);
  RUN_TEST(test_negotiate_username_should_send_username_length_followed_by_username_to_server, 91);
  RUN_TEST(test_negotiate_username_should_return_FAILURE_if_timer_expires, 95);
  RUN_TEST(test_negotiate_username_should_return_INVALID_if_server_replies_with_I, 113);
  RUN_TEST(test_negotiate_username_should_return_INVALID_if_server_replies_with_T, 131);
  RUN_TEST(test_negotiate_username_should_return_SUCCESS_if_server_replies_with_Y, 149);

  return (UnityEnd());
}
