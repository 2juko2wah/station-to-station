#include "vec2.h"

#include <math.h>
#include <stdlib.h>

float Vec2SqMag(Vec2 a) {
    return a.x*a.x + a.y*a.y;
}

float Vec2Mag(Vec2 a) {
    return sqrtf(a.x*a.x + a.y*a.y);
}

float Vec2SqDist(Vec2 a, Vec2 b) {
    return (b.x * b.x - a.x * a.x) + (b.y * b.y - a.y * a.y);
}

float Vec2Dist(Vec2 a, Vec2 b) {
    return sqrtf((b.x * b.x - a.x * a.x) + (b.y * b.y - a.y * a.y));
}

Vec2 Vec2Norm(Vec2 a) {
    float mag = Vec2Mag(a);
    if (mag == 0.0f) {
        return Vec2(0, 0);
    }

    return a / mag;
}