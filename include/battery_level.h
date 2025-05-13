#ifndef BATTERY_LEVEL_H
#define BATTERY_LEVEL_H

namespace SwitchKit {
enum BatteryLevel
{
    BATTERY_FULL = 8,
    BATTERY_MEDIUM = 6,
    BATTERY_LOW = 4,
    BATTERY_CRITICAL = 2,
    BATTERY_EMPTY = 0
};
}
#endif // BATTERY_LEVEL_H
