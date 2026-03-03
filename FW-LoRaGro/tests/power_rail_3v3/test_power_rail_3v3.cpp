#include <zephyr/ztest.h>
#include "power_rail_3v3.hpp"

ZTEST(power_3v3_suite, test_power_3v3_enable_disable)
{
    loragro::PowerRail3V3 rail;
    rail.powerOn();
    zassert_true(rail.isOn(), "Rail should be on");
    rail.powerOff();
    zassert_false(rail.isOn(), "Rail should be off");
}

ZTEST(power_3v3_suite, test_power_3v3_read_voltage)
{
    loragro::PowerRail3V3 rail;
    rail.powerOn();
    int voltage = rail.get_voltage();
    zassert_true(voltage > 0, "Voltage should be > 0");
}

ZTEST_SUITE(power_3v3_suite, NULL, NULL, NULL, NULL, NULL);