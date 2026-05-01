#ifndef VEC2_H
#define VEC2_H

#include <stdint.h>
#include <math.h>

#define DEG2RAD(DEG) (DEG * (3.14 / 180.0))
#define NORMDEG(DEG) fmod(fmod((DEG), 360.0f) + 360.0f, 360.0f)

struct Vec2 {
    float x;
    float y;

    Vec2(float x, float y) : x(x), y(y) {}

    Vec2() : x(0), y(0) {}

    Vec2 operator+ (Vec2 other) { return Vec2(this->x + other.x, this->y + other.y); }
    Vec2 operator- (Vec2 other) { return Vec2(this->x - other.x, this->y - other.y); }

    Vec2 operator* (float s) { return Vec2(this->x * s, this->y * s); }
    Vec2 operator/ (float s) { return Vec2(this->x / s, this->y / s); }

    static Vec2 fromDeg(float deg) {
        float ndeg = NORMDEG(deg);
        return Vec2(cosf(DEG2RAD(ndeg)), sinf(DEG2RAD(ndeg)));
    }
};

float Vec2SqMag(Vec2 a);
float Vec2Mag(Vec2 a);

float Vec2SqDist(Vec2 a, Vec2 b);
float Vec2Dist(Vec2 a, Vec2 b);

Vec2 Vec2Norm(Vec2 a);

#endif