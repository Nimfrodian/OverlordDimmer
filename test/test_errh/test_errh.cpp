#include "unity.h"
#include "errh.h"

// function mocks system time function
static int64_t ti_us_currTimeMock_S64 = 0;
int64_t sysTimerFuncMock(void)
{
    ti_us_currTimeMock_S64++;
    return ti_us_currTimeMock_S64;
}

void setUp(void)
{
    tTIMH_INITDATA_STR timh_cfgData_str = {0};
    timh_cfgData_str.sysTimeFunc = sysTimerFuncMock;
    timh_init(&timh_cfgData_str);
}

void tearDown(void)
{
}

void test_function_triggerOneTypeOfError(void)
{
    // check no pre-existing errors exist
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, errh_readReportedErrorCount(), "number of reported errors is not 0");

    int64_t ti_us_startTimestamp_S64 = ti_us_currTimeMock_S64;
    // check that repeating errors only get their count and timestamp updated
    for (uint16_t i_U16 = 0; i_U16 < 1000; i_U16++)
    {
        // report the same error
        errh_reportError(ERRH_ERROR_CRITICAL, 2, 3, 4, 5);

        // check that error was correctly stored
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, errh_readReportedErrorCount(), "number of reported errors is not correct");

        tERRH_ERRORDATA_STR triggeredError_str = errh_readError(0);
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, triggeredError_str.moduleId, "moduleId Mismatch");
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, triggeredError_str.instanceId, "instanceId Mismatch");
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(4, triggeredError_str.apiId, "apiId Mismatch");
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, triggeredError_str.errorId, "errorId Mismatch");
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(ERRH_ERROR_CRITICAL, triggeredError_str.errorLvl, "errorLvl Mismatch");
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(i_U16 + ti_us_startTimestamp_S64 + 1, triggeredError_str.ti_us_timestamp, "ti_us_timestamp Mismatch");
        uint16_t expectedCount_U16 = ((i_U16+1) > 255)? 255 : (i_U16 + 1);
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(expectedCount_U16, triggeredError_str.count_U8, "count_U8 Mismatch");
    }

    errh_clearErrorCount();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, errh_readReportedErrorCount(), "number of reported errors is not cleared");
}

void test_function_triggerSeveralTypesOfErrors(void)
{
    // check no pre-existing errors exist
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, errh_readReportedErrorCount(), "number of reported errors is not 0");

    int64_t ti_us_startTimestamp_S64 = ti_us_currTimeMock_S64;
    // check that unique errors are logged
    for (uint16_t i_U16 = 0; i_U16 < (ERRH_NR_ERROR_BUFFER_SIZE_U32 - 1); i_U16++)
    {
        // report the different error
        errh_reportError(ERRH_ERROR_CRITICAL, 2, 3, 4, i_U16);

        // check that error was correctly stored
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(i_U16 + 1, errh_readReportedErrorCount(), "number of reported errors is not correct");

        tERRH_ERRORDATA_STR triggeredError_str = errh_readError(i_U16);
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, triggeredError_str.moduleId, "moduleId Mismatch");
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, triggeredError_str.instanceId, "instanceId Mismatch");
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(4, triggeredError_str.apiId, "apiId Mismatch");
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(i_U16, triggeredError_str.errorId, "errorId Mismatch");
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(ERRH_ERROR_CRITICAL, triggeredError_str.errorLvl, "errorLvl Mismatch");
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(i_U16 + ti_us_startTimestamp_S64 + 1, triggeredError_str.ti_us_timestamp, "ti_us_timestamp Mismatch");
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, triggeredError_str.count_U8, "count_U8 Mismatch");
    }

    // Memory should be full, trigger another error and note if memory full is logged instead
    // report another error
    errh_reportError(ERRH_ERROR_CRITICAL, 2, 3, 4, -2);
    // check that memory full error was correctly stored
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(ERRH_NR_ERROR_BUFFER_SIZE_U32, errh_readReportedErrorCount(), "number of reported errors is not correct");
    tERRH_ERRORDATA_STR triggeredError_str = errh_readError(ERRH_NR_ERROR_BUFFER_SIZE_U32 - 1);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(-1, triggeredError_str.moduleId, "moduleId Mismatch");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(-1, triggeredError_str.instanceId, "instanceId Mismatch");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(-1, triggeredError_str.apiId, "apiId Mismatch");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(-1, triggeredError_str.errorId, "errorId Mismatch");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(ERRH_WARNING, triggeredError_str.errorLvl, "errorLvl Mismatch");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(ERRH_NR_ERROR_BUFFER_SIZE_U32 + ti_us_startTimestamp_S64, triggeredError_str.ti_us_timestamp, "ti_us_timestamp Mismatch");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, triggeredError_str.count_U8, "count_U8 Mismatch");

    errh_clearErrorCount();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, errh_readReportedErrorCount(), "number of reported errors is not cleared");
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_function_triggerOneTypeOfError);
  RUN_TEST(test_function_triggerSeveralTypesOfErrors);
  return UNITY_END();
}

int main(void) {
  return runUnityTests();
}