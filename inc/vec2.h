#ifndef VEC2_H
#define VEC2_H

#define DEG2RAD(DEG) (DEG * (3.14 / 180.0))

typedef struct {
    float x;
    float y;
} Vec2;

Vec2 Vec2New(float x, float y);
Vec2 Vec2Zero();

Vec2 Vec2Add(Vec2 a, Vec2 b);
Vec2 Vec2Sub(Vec2 a, Vec2 b);

Vec2 Vec2Scale(Vec2 a, float s);

float Vec2SqMag(Vec2 a);
float Vec2Mag(Vec2 a);

float Vec2SqDist(Vec2 a, Vec2 b);
float Vec2Dist(Vec2 a, Vec2 b);

Vec2 Vec2Norm(Vec2 a);

Vec2 Vec2FromDeg(float deg);
float DegNorm(float deg);

#endif