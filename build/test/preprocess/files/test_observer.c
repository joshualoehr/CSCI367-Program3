#include "prog3_observer.h"
#include "unity.h"


















void test_observer_should_fail_with_invalid_arguments(void) {

 int argc, ret_code;

 char *argv[3];



 argc = 2;

 ret_code = main_observer(argc, argv);

 UnityAssertEqualNumber((UNITY_INT)((

1

)), (UNITY_INT)((ret_code)), (

((void *)0)

), (UNITY_UINT)(20), UNITY_DISPLAY_STYLE_INT);



 argc = 3;

 ret_code = main_observer(argc, argv);

 UnityAssertEqualNumber((UNITY_INT)((

0

)), (UNITY_INT)((ret_code)), (

((void *)0)

), (UNITY_UINT)(24), UNITY_DISPLAY_STYLE_INT);



 argc = 4;

 ret_code = main_observer(argc, argv);

 UnityAssertEqualNumber((UNITY_INT)((

1

)), (UNITY_INT)((ret_code)), (

((void *)0)

), (UNITY_UINT)(28), UNITY_DISPLAY_STYLE_INT);

}
