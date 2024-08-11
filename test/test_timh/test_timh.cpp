#include "unity.h"
#include "timh.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_function_updateTime(void)
{
    tTIMH_TIMEDATA_STR timeData_str = {0};

    // test first CAN message
    uint8_t canMsgData0[7] = {0xE8, 0x07, 0x08, 0x0B, 0x0E, 0x25, 0x3B};
    uint32_t canMsgId0 = 0x94;

    timh_canMsgParse_ev(canMsgData0, &canMsgId0);

    timeData_str = timh_ti_readCurrentTime();

    TEST_ASSERT_EQUAL_UINT16(2024, timeData_str.year_U16);
    TEST_ASSERT_EQUAL_UINT8( 8, timeData_str.month_U8);
    TEST_ASSERT_EQUAL_UINT8(11, timeData_str.day_U8);
    TEST_ASSERT_EQUAL_UINT8(14, timeData_str.hour_U8);
    TEST_ASSERT_EQUAL_UINT8(37, timeData_str.minute_U8);
    TEST_ASSERT_EQUAL_UINT8(59, timeData_str.second_U8);

    // test update CAN message
    uint8_t canMsgData1[7] = {0xE8, 0x07, 0x08, 0x0B, 0x0E, 0x26, 0x00};
    uint32_t canMsgId1 = 0x94;

    timh_canMsgParse_ev(canMsgData1, &canMsgId1);

    timeData_str = timh_ti_readCurrentTime();

    TEST_ASSERT_EQUAL_UINT16(2024, timeData_str.year_U16);
    TEST_ASSERT_EQUAL_UINT8( 8, timeData_str.month_U8);
    TEST_ASSERT_EQUAL_UINT8(11, timeData_str.day_U8);
    TEST_ASSERT_EQUAL_UINT8(14, timeData_str.hour_U8);
    TEST_ASSERT_EQUAL_UINT8(38, timeData_str.minute_U8);
    TEST_ASSERT_EQUAL_UINT8( 0, timeData_str.second_U8);
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_function_updateTime);
  return UNITY_END();
}

int main(void) {
  return runUnityTests();
}