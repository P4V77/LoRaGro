#pragma once

namespace loragro
{
    class PowerRail3V3
    {
    public:
        int powerOn();
        int powerOff();
        const bool isOn() const { return powered_; }

    private:
        bool powered_ = false;
        bool always_on_ = false;
    };

};