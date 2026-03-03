#pragma once

namespace loragro
{
    class PowerRail3V3
    {
    public:
        int powerOn();
        int powerOff();
        const bool isOn() const { return powered_; }

        int get_voltage() const
        {
            return powered_ ? 3300 : 0; // simulace 3.3V railu
        }

    private:
        bool powered_ = false;
        bool always_on_ = false;
    };

};