#ifndef AGENT_HEADER
#define AGENT_HEADER

#include <stdint.h>

#include "vec2.h"
#include "defns.h"

typedef struct {
    Vec2 position;

    Vec2 front_left;
    Vec2 front;
    Vec2 front_right;

    float heading;
} Agent;

typedef struct {
    uint8_t trail[WIDTH][HEIGHT];

    uint8_t num_stations;
    uint8_t cap_stations;

    
    uint64_t num_agents;
    uint64_t cap_agents;

    Vec2 *stations;
    Agent *agents;
} Field;

void InitAgent(Agent *agent);
void UpdateAgent(Agent *agent, Field *field, float dt);

void InitField(Field *field);
void UpdateField(Field *field, float dt);

void PlaceStation(Field *field, uint64_t x, uint64_t y);
void RemoveStation(Field *field, uint64_t x, uint64_t y);

void UpdateStations(Field *field, float dt);


#endif