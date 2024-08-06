#include "unity.h"
#include "lgic.h"

#define NUM_OF_TRIGGERS 10


void setUp(void)
{
    logicConfigType logicCfg;
    logicCfg.numOfTriggers = NUM_OF_TRIGGERS + 1; // 1 starting state + 10, 1 for each trigger
    init_logic(&logicCfg);
}

void tearDown(void) {
  // clean stuff up here
}

void test_function_delayTiming(void) {
  // test stuff

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