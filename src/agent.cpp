#include "agent.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL.h>

#include "vec2.h"
#include "defns.h"

void InitAgent(Agent *agent) {
    agent->heading  = SDL_randf() * 360.0f;
    agent->position = Vec2New(WIDTH / 2.0f, HEIGHT / 2.0f);
}

float SampleTrail(Field *field, Vec2 at) {
    int x = (int)at.x;
    int y = (int)at.y;

    if (x >= WIDTH || x < 0.0f || y >= HEIGHT || y < 0.0f) {
        return 0.0f;
    } 
    
    return field->trail[x][y];
}

void ReflectOnBounds(Agent *agent) {
    if (agent->position.y < 0.0f || agent->position.y >= HEIGHT) {
        agent->position.y = SDL_clamp(agent->position.y, 0, HEIGHT-1);
        agent->heading = DegNorm(360.0f - agent->heading);
    }

    if (agent->position.x < 0.0f || agent->position.x >= WIDTH) {
        agent->position.x = SDL_clamp(agent->position.x, 0, WIDTH-1);
        agent->heading = DegNorm(180.0f - agent->heading);
    }
}

void UpdateAgent(Agent *agent, Field *field, float dt) {
    ReflectOnBounds(agent);
    
    Vec2 fwd = Vec2Add(agent->position, Vec2Scale(Vec2FromDeg(DegNorm(agent->heading)), SENSOR_OFFSET));
    Vec2 lft = Vec2Add(agent->position, Vec2Scale(Vec2FromDeg(DegNorm(agent->heading - SENSOR_ANGLE)), SENSOR_OFFSET));
    Vec2 rgt = Vec2Add(agent->position, Vec2Scale(Vec2FromDeg(DegNorm(agent->heading + SENSOR_ANGLE)), SENSOR_OFFSET));

    float sfwd = SampleTrail(field, fwd);
    float slft = SampleTrail(field, lft);
    float srgt = SampleTrail(field, rgt);

    if (slft > sfwd && slft > srgt) {
        agent->heading -= HEADING_SPEED;
    } else if (srgt > sfwd && srgt > slft) {
        agent->heading += HEADING_SPEED;
    } else {
        agent->heading += (2.0f * (SDL_randf() - 0.5f)) * RANDOMNESS;
    }

    field->trail[(int)agent->position.x][(int)agent->position.y] = DEPOSIT;
    agent->position = Vec2Add(agent->position, Vec2Scale(Vec2FromDeg(agent->heading), AGENT_SPEED * dt));
}

void InitField(Field *field) {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            field->trail[x][y] = 0;
        }
    }

    field->num_stimuli = 0;
    field->cap_stimuli = STATION_CAPACITY;

    field->agents  = (Agent *)SDL_malloc(sizeof(Agent) * AGENT_CAPACITY);
    field->stimuli = (Stimulus *)SDL_malloc(sizeof(Stimulus) * STATION_CAPACITY);
}

void UpdateField(Field *field, float dt) {
    int16_t buffer[WIDTH][HEIGHT];
    
    SDL_memcpy(buffer, field->trail, sizeof(buffer));

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            uint8_t num_edges = 4;

            num_edges -= (x+1 >= WIDTH) + (x-1 < 0) + (y+1 >= HEIGHT) + (y-1 < 0);

            buffer[x][y] = SDL_clamp(buffer[x][y], 0, 255);
            buffer[x][y] = (field->trail[x][y-1]*(y-1 >= 0) + field->trail[x-1][y]*(x-1 >= 0) + field->trail[x][y] + field->trail[x+1][y]*(x+1 < WIDTH) + field->trail[x][y+1]*(y+1 < HEIGHT)) / (num_edges + 1);

            buffer[x][y] *= DECAY;
        }
    }

    SDL_memcpy(field->trail, buffer, (sizeof(buffer)));
}

// these could use the UpdateAgent treatment from before
int PlaceStimulus(Field *field, Vec2 pos, int16_t strength) {
    if (field->num_stimuli >= field->cap_stimuli) {
        field->cap_stimuli *= 2;
        field->stimuli = (Stimulus *)SDL_realloc(field->stimuli, sizeof(Stimulus) * field->cap_stimuli);

        if (!field->stimuli) {
            printf("[ERROR]: Array resize failed\n");
            exit(1);
        }
    }

    for (int i = 0; i < field->num_stimuli; ++i) {
        if (IN_BOUNDS(pos.x, field->stimuli[i].position.x - STATION_WIDTH, field->stimuli[i].position.x + STATION_WIDTH) && IN_BOUNDS(pos.y, field->stimuli[i].position.y - STATION_HEIGHT, field->stimuli[i].position.y + STATION_HEIGHT)) { 
            return 1;
        }
    }

    field->stimuli[field->num_stimuli].position = pos;
    field->stimuli[field->num_stimuli].strength = strength;

    field->num_stimuli++;

    return 0;
}

void RemoveStimulus(Field *field, Vec2 pos) {
    for (int i = 0; i < field->num_stimuli; ++i) {
        if (IN_BOUNDS(pos.x, field->stimuli[i].position.x - STATION_WIDTH / 2, field->stimuli[i].position.x + STATION_WIDTH / 2) && IN_BOUNDS(pos.y, field->stimuli[i].position.y - STATION_HEIGHT / 2, field->stimuli[i].position.y + STATION_HEIGHT / 2)) { 
            field->stimuli[i] = field->stimuli[--field->num_stimuli];
            
        }
    }
}

void UpdateStimuli(Field *field, float dt) {
    for (int i = 0; i < field->num_stimuli; ++i) {
        Vec2 pos = field->stimuli[i].position;
        for (int dy = -(STATION_HEIGHT / 2.0f); dy <= (STATION_HEIGHT / 2.0f); ++dy) {
            for (int dx = -(STATION_WIDTH / 2.0f); dx <= (STATION_WIDTH / 2.0f); ++dx) {
                if (pos.x + dx >= WIDTH || pos.x + dx < 0) continue;
                if (pos.y + dy >= HEIGHT || pos.y + dy < 0) continue;

                field->trail[(int)(pos.x + dx)][(int)(pos.y + dy)] = field->stimuli[i].strength;
            }
        }
    }
}