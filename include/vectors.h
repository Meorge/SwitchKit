#ifndef VECTORS_H
#define VECTORS_H

struct Vector2 {
    double x;
    double y;

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
};

#endif // VECTORS_H
