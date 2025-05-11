#include "imu.h"
#include <math.h>

namespace SwitchKit {
Vector3 IMUPacket::get_accel() const {
    // TODO: Set deadzone based on precision.
    Vector3 out;
    out.x = abs(accel_x) > 205 ? (accel_x * G_RANGE / SENSOR_RES / 1000.0) : 0;
    out.y = abs(accel_y) > 205 ? (accel_y * G_RANGE / SENSOR_RES / 1000.0) : 0;
    out.z = abs(accel_z) > 205 ? (accel_z * G_RANGE / SENSOR_RES / 1000.0) : 0;
    return out;
}

Vector3 IMUPacket::get_gyro() const {
    // TODO: Set deadzone based on precision.
    Vector3 out;
    out.x = abs(gyro_1) > 75 ? (gyro_1 * G_GAIN / SENSOR_RES) : 0;
    out.y = abs(gyro_2) > 75 ? (gyro_2 * G_GAIN / SENSOR_RES) : 0;
    out.z = abs(gyro_3) > 75 ? (gyro_3 * G_GAIN / SENSOR_RES) : 0;
    return out;
}
}
