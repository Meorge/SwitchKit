#ifndef VECTOR2_H
#define VECTOR2_H

struct Vector2 {
    double x;
    double y;

    Vector2(double p_x, double p_y) {
        x = p_x;
        y = p_y;
    }
};

#endif // VECTOR2_H