#include <zephyr/ztest.h>

#include "sensors/co2_sensor_adapter.hpp"
#include "data_types.hpp"

static const struct device *const co2_sensor_dev =
    DEVICE_DT_GET(DT_ALIAS(co2_sensor));

struct co2_sensor_adapter_suite_fixture
{
    loragro::CO2SensorAdapter *co2_sensor;
};

static struct co2_sensor_adapter_suite_fixture g_fixture;

static void *co2_sensor_adapter_suite_setup(void)
{
    static loragro::CO2SensorAdapter co2_sensor(
        co2_sensor_dev,
        loragro::SensorID::CO2_CONC,
        loragro::SensorID::CO2_TEMP,
        loragro::SensorID::CO2_RH);

    int ret = co2_sensor.init();
    zassert_equal(ret, 0, "Init of CO2 Sensor failed");

    g_fixture.co2_sensor = &co2_sensor;
    return &g_fixture;
}

ZTEST_SUITE(co2_sensor_adapter_suite, NULL, co2_sensor_adapter_suite_setup, NULL, NULL, NULL);

ZTEST_F(co2_sensor_adapter_suite, test_co2_init)
{
    zassert_not_null(fixture->co2_sensor, "Fixture co2_sensor is NULL");
}

ZTEST_F(co2_sensor_adapter_suite, test_co2_sample)
{
    int ret = fixture->co2_sensor->sample();
    zassert_equal(ret, 0, "sample() failed: %d", ret);

    int co2 = fixture->co2_sensor->get_measurements()[0].value.val1;
    int temp = fixture->co2_sensor->get_measurements()[1].value.val1;
    int hum = fixture->co2_sensor->get_measurements()[2].value.val1;

    zassert_true(co2 >= 400 && co2 <= 5000, "CO2 out of range: %d ppm", co2);
    zassert_true(temp >= 0 && temp <= 50, "Temp out of range: %d C", temp);
    zassert_true(hum >= 0 && hum <= 100, "Humidity out of range: %d%%", hum);
}