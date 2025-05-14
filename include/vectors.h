#ifndef VECTORS_H
#define VECTORS_H

namespace SwitchKit {
struct Vector2 {
    double x;
    double y;

    Vector2() {
        x = 0.0;
        y = 0.0;
    }
    
    Vector2(double p_x, double p_y) {
        x = p_x;
        y = p_y;
    }
};

struct Vector3 {
    double x;
    double y;
    double z;

    Vector3() {
        x = 0.0;
        y = 0.0;
        z = 0.0;
    }
    
    Vector3(double p_x, double p_y, double p_z) {
        x = p_x;
        y = p_y;
        z = p_z;
    }

    Vector3 operator*(const Vector3 &rhs) {
        Vector3 out;
        out.x = x * rhs.x;
        out.y = y * rhs.y;
        out.z = z * rhs.z;
        return out;
    }

    Vector3 operator+=(Vector3 &rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return Vector3(x, y, z);
    }

    Vector3 operator+(Vector3 &rhs) {
        Vector3 out;
        out.x = x + rhs.x;
        out.y = y + rhs.y;
        out.z = z + rhs.z;
        return out;
    }

    Vector3 operator*=(const double &rhs) {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return Vector3(x, y, z);
    }

    Vector3 operator*(const double &rhs) {
        Vector3 out;
        out.x = x * rhs;
        out.y = y * rhs;
        out.z = z * rhs;
        return out;
    }
};
}

#endif // VECTORS_H
