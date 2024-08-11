#include "unity.h"
#include "lgic.h"

#define NUM_OF_TRIGGERS 10


void setUp(void)
{
    tLGIC_INITDATA_STR lgicCfg;
    lgicCfg.nr_numOfPhysicalOutputs = NUM_OF_TRIGGERS;
    lgic_init(&lgicCfg);
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