#include "unity.h"
#include "logic.h"
#include "logic.c"

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void test_function_delayTiming(void) {
  // test stuff
  calc_duty_cycle();

  TEST_ASSERT(1);
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_function_delayTiming);
  return UNITY_END();
}

int main(void) {
  return runUnityTests();
}