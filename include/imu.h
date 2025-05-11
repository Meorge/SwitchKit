#ifndef IMU_H
#define IMU_H

#define G_RANGE 16000
#define G_GAIN 4588 // 4000 + 15% to avoid saturation, according to documentation
#define SENSOR_RES 65535 // Don't fully understand this...

#include <cstdint>

#include "vectors.h"

namespace SwitchKit {
struct IMUPacket {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;

    int16_t gyro_1;
    int16_t gyro_2;
    int16_t gyro_3;

    Vector3 get_accel() const;
    Vector3 get_gyro() const;
};
}

#endif // IMU_H
