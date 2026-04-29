#include "vec2.h"

#include <math.h>
#include <stdlib.h>

Vec2 Vec2New(float x, float y) {
    return (Vec2) { .x=x, .y=y };
}

Vec2 Vec2Zero() {
    return (Vec2) { .x=0.0f, .y=0.0f };
}

Vec2 Vec2Add(Vec2 a, Vec2 b) {
    return (Vec2) { .x=a.x + b.x, .y=a.y + b.y };
}

Vec2 Vec2Sub(Vec2 a, Vec2 b) {
    return (Vec2) { .x=a.x - b.x, .y=a.y - b.y };
}

Vec2 Vec2Scale(Vec2 a, float s) {
    return (Vec2) { .x=a.x*s, .y=a.y*s };
}

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
        return (Vec2) { .x = 0.0f, .y = 0.0f };
    }

    return Vec2Scale(a, 1.0f / mag);
}

Vec2 Vec2FromDeg(float deg) {
    return (Vec2) { .x = cosf(DEG2RAD(deg)), .y = sinf(DEG2RAD(deg)) };
}

float DegNorm(float deg) {
    return fmod(fmod(deg, 360.0f) + 360.0f, 360.0f);
}