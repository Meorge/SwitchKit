#ifndef IMU_H
#define IMU_H

#define G_RANGE 16000
#define G_GAIN 4588 // 4000 + 15% to avoid saturation, according to documentation
#define SENSOR_RES 65535 // Don't fully understand this...

#include <cstdint>

#include "vectors.h"

namespace SwitchKit {
struct IMUCalibrationData {
    int16_t acc_x_stable;
    int16_t acc_y_stable;
    int16_t acc_z_stable;

    int16_t acc_x_coeff;
    int16_t acc_y_coeff;
    int16_t acc_z_coeff;

    int16_t gyro_x_stable;
    int16_t gyro_y_stable;
    int16_t gyro_z_stable;

    int16_t gyro_x_coeff;
    int16_t gyro_y_coeff;
    int16_t gyro_z_coeff;
};

struct IMUPacket {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;

    int16_t gyro_1;
    int16_t gyro_2;
    int16_t gyro_3;

    Vector3 get_accel() const;
    Vector3 get_gyro(const IMUCalibrationData &p_calib) const;
};
}

#endif // IMU_H
