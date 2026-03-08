#include <zephyr/ztest.h>
#include "sensors/soil_capacitive_adapter.hpp"
#include "data_types.hpp"

#define ADC_NODE DT_NODELABEL(adc0)

struct soil_capacitive_adapter_suite_fixture
{
    loragro::SoilCapacitiveSensor *soil;
};

static struct soil_capacitive_adapter_suite_fixture g_fixture;

static void *soil_capacitive_adapter_suite_setup(void)
{
    static loragro::SoilCapacitiveSensor soil(
        DEVICE_DT_GET(ADC_NODE), loragro::SensorID::SOIL_ANALOG_MOISTURE);

    int ret = soil.init();
    zassert_equal(ret, 0, "init() failed: %d", ret);

    g_fixture.soil = &soil;
    return &g_fixture;
}

ZTEST_SUITE(soil_capacitive_adapter_suite, NULL, soil_capacitive_adapter_suite_setup, NULL, NULL, NULL);

ZTEST_F(soil_capacitive_adapter_suite, test_init)
{
    zassert_not_null(fixture->soil, "Soil is NULL");
}

ZTEST_F(soil_capacitive_adapter_suite, test_is_connected)
{
    int ret = fixture->soil->is_connected();
    zassert_equal(ret, 0, "is_connected() failed: %d", ret);
}

ZTEST_F(soil_capacitive_adapter_suite, test_sample, test_sample)
{
    for (size_t i = 0; i < 25; i++)
    {
        int ret = fixture->soil->sample();
        zassert_equal(ret, 0, "sample() failed: %d", ret);

        int mv = fixture->soil->get_measurements()[0].val.val1;
        zassert_equal(mv > 1000, "Measurement out of range, too low: %d", mv);
        zassert_equal(mv < 5000, "Measurement out of range, too high: %d", mv);
    }
}
