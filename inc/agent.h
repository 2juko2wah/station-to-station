#ifndef AGENT_HEADER
#define AGENT_HEADER

#include <stdint.h>

#include "vec2.h"
#include "defns.h"

// [-255, 255]
typedef int16_t scalar;

typedef struct {
    Vec2 position;
    float heading;
} Agent;

typedef struct {
    Vec2 position;
    scalar strength;
} Stimulus;

// [TODO]: Really I just gotta get rid of this
typedef struct {
    int16_t trail[WIDTH][HEIGHT]; // this will eventually get its own static/global scope

    uint8_t num_stimuli;
    uint8_t cap_stimuli;

    Stimulus *stimuli;
    Agent *agents;
} Field;

void InitAgent(Agent *agent);
void UpdateAgent(Agent *agent, Field *field, float dt);

void InitField(Field *field);
void UpdateField(Field *field, float dt);

int PlaceStimulus(Field *field, Vec2 pos, int16_t strength);
void RemoveStimulus(Field *field, Vec2 pos);

void UpdateStimuli(Field *field, float dt);

#endif