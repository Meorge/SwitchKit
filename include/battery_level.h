#ifndef BATTERY_LEVEL_H
#define BATTERY_LEVEL_H

namespace SwitchKit {
enum BatteryLevel
{
    FULL = 8,
    MEDIUM = 6,
    LOW = 4,
    CRITICAL = 2,
    EMPTY = 0
};
}
#endif // BATTERY_LEVEL_H
