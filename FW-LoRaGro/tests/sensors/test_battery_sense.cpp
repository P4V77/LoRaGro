#include <zephyr/ztest.h>
#include "sensors/battery_sense.hpp"

#define ADC_NODE DT_NODELABEL(adc0)

struct battery_sense_suite_fixture
{
    loragro::BatterySenseAdapter *bat;
};

static struct battery_sense_suite_fixture g_fixture;

static void *battery_suite_setup(void)
{
    static loragro::BatterySenseAdapter bat(
        DEVICE_DT_GET(ADC_NODE), 0x40);

    int ret = bat.init();
    zassert_equal(ret, 0, "init() failed: %d", ret);

    g_fixture.bat = &bat;
    return &g_fixture;
}

ZTEST_SUITE(battery_sense_suite, NULL, battery_suite_setup, NULL, NULL, NULL);

ZTEST_F(battery_sense_suite, test_init)
{
    zassert_not_null(fixture->bat, "bat is NULL");
}

ZTEST_F(battery_sense_suite, test_is_connected)
{
    int ret = fixture->bat->is_connected();
    zassert_equal(ret, 0, "is_connected() failed: %d", ret);
}

ZTEST_F(battery_sense_suite, test_sample_returns_valid_voltage)
{
    int ret = fixture->bat->sample();
    zassert_equal(ret, 0, "sample() failed: %d", ret);

    int32_t mv = fixture->bat->get_measurements()[0].value.val1;
    zassert_true(mv > 1000, "Voltage too low: %d mV", mv);
    zassert_true(mv < 5000, "Voltage too high: %d mV", mv);
}

ZTEST_F(battery_sense_suite, test_sample_ramp)
{
    for (int i = 0; i < 5; i++)
    {
        int ret = fixture->bat->sample();
        zassert_equal(ret, 0, "sample() failed iter %d: %d", i, ret);

        int32_t mv = fixture->bat->get_measurements()[0].value.val1;
        zassert_true(mv > 1000, "Too low iter %d: %d mV", i, mv);
        zassert_true(mv < 5000, "Too high iter %d: %d mV", i, mv);
    }
}