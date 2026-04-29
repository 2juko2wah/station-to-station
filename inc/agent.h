#ifndef AGENT_HEADER
#define AGENT_HEADER

#include <stdint.h>

#include "vec2.h"
#include "defns.h"

typedef struct {
    Vec2 position;
    float heading;
} Agent;

typedef struct {
    int16_t trail[WIDTH][HEIGHT];

    uint8_t num_attractors;
    uint8_t cap_attractors;

    uint8_t num_deflectors;
    uint8_t cap_deflectors;


    Vec2 *attractors;
    Vec2 *deflectors;
    Agent *agents;
} Field;

void InitAgent(Agent *agent);
void UpdateAgent(Agent *agent, Field *field, float dt);

void InitField(Field *field);
void UpdateField(Field *field, float dt);

void PlaceAttractor(Field *field, uint64_t x, uint64_t y);
void RemoveAttractor(Field *field, uint64_t x, uint64_t y);


void UpdateAttractors(Field *field, float dt);


void PlaceDeflector(Field *field, uint64_t x, uint64_t y);
void RemoveDeflector(Field *field, uint64_t x, uint64_t y);


void UpdateDeflectors(Field *field, float dt);

#endif