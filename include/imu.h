#ifndef IMU_H
#define IMU_H

#define G_RANGE 16000
#define G_GAIN 4588 // 4000 + 15% to avoid saturation, according to documentation
#define SENSOR_RES 65535 // Don't fully understand this...

#include <cstdint>

#include "vectors.h"

namespace SwitchKit {
struct IMUCalibrationData {
    int16_t cal_acc_origin_x;
    int16_t cal_acc_origin_y;
    int16_t cal_acc_origin_z;

    int16_t cal_acc_horizontal_offset_x;
    int16_t cal_acc_horizontal_offset_y;
    int16_t cal_acc_horizontal_offset_z;

    int16_t cal_gyro_offset_x;
    int16_t cal_gyro_offset_y;
    int16_t cal_gyro_offset_z;

    int16_t cal_gyro_coeff_x;
    int16_t cal_gyro_coeff_y;
    int16_t cal_gyro_coeff_z;
};

struct IMUPacket {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;

    int16_t gyro_1;
    int16_t gyro_2;
    int16_t gyro_3;

    Vector3 get_accel() const;
    Vector3 get_gyro(const IMUCalibrationData *p_calib) const;
};
}

#endif // IMU_H
