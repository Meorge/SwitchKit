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
    out.x = abs(gyro_1) > 75 ? (gyro_1 * G_GAIN / SENSOR_RES) : 0;
    out.y = abs(gyro_2) > 75 ? (gyro_2 * G_GAIN / SENSOR_RES) : 0;
    out.z = abs(gyro_3) > 75 ? (gyro_3 * G_GAIN / SENSOR_RES) : 0;
    // out.x = abs(gyro_1) > 75 ? gyro_1 : 0;
    // out.y = abs(gyro_2) > 75 ? gyro_2 : 0;
    // out.z = abs(gyro_3) > 75 ? gyro_3 : 0;

    // Apply calibration.
    double gyro_cal_coeff_x = 936.0 / (float)(p_calib.gyro_x_coeff - p_calib.gyro_x_stable);
    double gyro_cal_coeff_y = 936.0 / (float)(p_calib.gyro_y_coeff - p_calib.gyro_y_stable);
    double gyro_cal_coeff_z = 936.0 / (float)(p_calib.gyro_z_coeff - p_calib.gyro_z_stable);

    // printf("cal gyro coeff = %d\n", p_calib.gyro_x_coeff);
    // out.x = (out.x - p_calib.gyro_x_stable) * gyro_cal_coeff_x;
    // out.y = (out.y - p_calib.gyro_y_stable) * gyro_cal_coeff_y;
    // out.z = (out.z - p_calib.gyro_z_stable) * gyro_cal_coeff_z;
    return out;
}
}
