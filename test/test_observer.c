/*
 * test_observer.c
 *
 *  Created on: Feb 23, 2017
 *      Author: loehrj
 */

#include <stdio.h>
#include <stdlib.h>
#include <unity.h>

#include "prog3_observer.h"

void test_observer_should_fail_with_invalid_arguments(void) {
	int argc, ret_code;
	char *argv[3];

	argc = 2;
	ret_code = main_observer(argc, argv);
	TEST_ASSERT_EQUAL_INT(EXIT_FAILURE, ret_code);

	argc = 3;
	ret_code = main_observer(argc, argv);
	TEST_ASSERT_EQUAL_INT(EXIT_SUCCESS, ret_code);

	argc = 4;
	ret_code = main_observer(argc, argv);
	TEST_ASSERT_EQUAL_INT(EXIT_FAILURE, ret_code);
}



