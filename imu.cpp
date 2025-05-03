#include "imu.h"

Vector3 get_accel(const IMUPacket &p_packet) {
    Vector3 out;
    out.x = p_packet.accel_x * G_RANGE / SENSOR_RES / 1000.0;
    out.y = p_packet.accel_y * G_RANGE / SENSOR_RES / 1000.0;
    out.z = p_packet.accel_z * G_RANGE / SENSOR_RES / 1000.0;
    return out;
}

Vector3 get_rotation_deg(const IMUPacket &p_packet) {
    Vector3 out;
    out.x = p_packet.gyro_1 * G_GAIN / SENSOR_RES;
    out.y = p_packet.gyro_2 * G_GAIN / SENSOR_RES;
    out.z = p_packet.gyro_3 * G_GAIN / SENSOR_RES;
    return out;
}

Vector3 get_rotation_rev(const IMUPacket &p_packet) {
    Vector3 out = get_rotation_deg(p_packet);
    out.x /= 360.0;
    out.y /= 360.0;
    out.z /= 360.0;
    return out;
}