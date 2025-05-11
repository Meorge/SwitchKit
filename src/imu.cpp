#include "imu.h"
#include <math.h>
#include <cstring>
#include <stdio.h>

namespace SwitchKit {

// From https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/spi_flash_notes.md.
int16_t uint16_to_int16(uint16_t a) {
	int16_t b;
	char* aPointer = (char*)&a, *bPointer = (char*)&b;
	memcpy(bPointer, aPointer, sizeof(a));
	return b;
}

Vector3 IMUPacket::get_accel() const {
    // TODO: Set deadzone based on precision.
    Vector3 out;
    out.x = abs(accel_x) > 205 ? (accel_x * G_RANGE / SENSOR_RES / 1000.0) : 0;
    out.y = abs(accel_y) > 205 ? (accel_y * G_RANGE / SENSOR_RES / 1000.0) : 0;
    out.z = abs(accel_z) > 205 ? (accel_z * G_RANGE / SENSOR_RES / 1000.0) : 0;
    return out;
}

Vector3 IMUPacket::get_gyro(const IMUCalibrationData &p_calib) const {
    // TODO: Set deadzone based on precision.
    Vector3 out;
    int32_t deadzone = 75;

    // Basic no-calibration version.
    if (false) {
        out.x = abs(gyro_1) > deadzone ? (gyro_1 * G_GAIN / SENSOR_RES) : 0;
        out.y = abs(gyro_2) > deadzone ? (gyro_2 * G_GAIN / SENSOR_RES) : 0;
        out.z = abs(gyro_3) > deadzone ? (gyro_3 * G_GAIN / SENSOR_RES) : 0;
        return out;
    }

    // Factory calibration.
    else if (true) {
        double gyro_cal_coeff_x = 936.0 / (float)(p_calib.cal_gyro_coeff_x - uint16_to_int16(p_calib.cal_gyro_offset_x));
        double gyro_cal_coeff_y = 936.0 / (float)(p_calib.cal_gyro_coeff_y - uint16_to_int16(p_calib.cal_gyro_offset_y));
        double gyro_cal_coeff_z = 936.0 / (float)(p_calib.cal_gyro_coeff_z - uint16_to_int16(p_calib.cal_gyro_offset_z));

        if (abs(gyro_1) > deadzone) {
            out.x = (gyro_1 - uint16_to_int16(p_calib.cal_gyro_offset_x)) * gyro_cal_coeff_x;
        } else {
            out.x = 0;
        }

        if (abs(gyro_2) > deadzone) {
            out.y = (gyro_2 - uint16_to_int16(p_calib.cal_gyro_offset_y)) * gyro_cal_coeff_y;
        } else {
            out.y = 0;
        }

        if (abs(gyro_3) > deadzone) {
            out.z = (gyro_3 - uint16_to_int16(p_calib.cal_gyro_offset_z)) * gyro_cal_coeff_z;
        } else {
            out.z = 0;
        }

        return out;
    }
}
}
