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

void UpdateAgent(Agent *agent, Field *field, float dt) {
    Vec2 front       = Vec2Add(agent->position, Vec2Scale(Vec2FromDeg(DegNorm(agent->heading)), SENSOR_OFFSET));
    Vec2 front_left  = Vec2Add(agent->position, Vec2Scale(Vec2FromDeg(DegNorm(agent->heading - SENSOR_ANGLE)), SENSOR_OFFSET));
    Vec2 front_right = Vec2Add(agent->position, Vec2Scale(Vec2FromDeg(DegNorm(agent->heading + SENSOR_ANGLE)), SENSOR_OFFSET));

    if (agent->position.y < 0.0f) {
        agent->position.y = 0.0f;
        agent->heading = DegNorm(360.0f - agent->heading);
    } else if (agent->position.y >= HEIGHT) {
        agent->position.y = HEIGHT - 1.0f;
        agent->heading = DegNorm(360.0f - agent->heading);
    }
    if (agent->position.x < 0.0f) {
        agent->position.x = 0.0f;
        agent->heading = DegNorm(180.0f - agent->heading);
    } else if (agent->position.x >= WIDTH) {
        agent->position.x = (float)WIDTH - 1.0f;
        agent->heading = DegNorm(180.0f - agent->heading);
        
    }

    float left    = field->trail[(int)front_left.x][(int)front_left.y]   * (front_left.x < WIDTH  && front_left.x >= 0.0f && front_left.y < HEIGHT && front_left.y >= 0.0f);
    float forward = field->trail[(int)front.x][(int)front.y]             * (front.x < WIDTH  && front.x >= 0.0f && front.y < HEIGHT && front.y >= 0.0f);
    float right   = field->trail[(int)front_right.x][(int)front_right.y] * (front_right.x < WIDTH  && front_right.x >= 0.0f && front_right.y < HEIGHT && front_right.y >= 0.0f);
    
    agent->heading -= HEADING_SPEED * (left > forward && left > right);
    agent->heading += HEADING_SPEED * (right > forward && right > left);
    agent->heading += (2.0f * (SDL_randf() - 0.5f)) * RANDOMNESS * (!(left > forward && left > right) + !(right > forward && right > left));

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

    field->agents  = SDL_malloc(sizeof(Agent) * AGENT_CAPACITY);
    field->stimuli = SDL_malloc(sizeof(Stimulus) * STATION_CAPACITY);
}

void UpdateField(Field *field, float dt) {
    int16_t buffer[WIDTH][HEIGHT];
    
    SDL_memcpy(buffer, field->trail, sizeof(buffer));

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            uint8_t num_edges = 4;

            num_edges -= (x+1 >= WIDTH) + (x-1 < 0) + (y+1 >= HEIGHT) + (y-1 < 0);

            buffer[x][y] = SDL_clamp(buffer[x][y], 0, 255);
            buffer[x][y] = (buffer[x][y-1]*(y-1 >= 0) + buffer[x-1][y]*(x-1 >= 0) + buffer[x][y] + buffer[x+1][y]*(x+1 < WIDTH) + buffer[x][y+1]*(y+1 < HEIGHT)) / (num_edges + 1);

            buffer[x][y] *= DECAY;
        }
    }

    SDL_memcpy(field->trail, buffer, (sizeof(buffer)));
}


void PlaceStimulus(Field *field, Vec2 pos, int16_t strength) {
    if (field->num_stimuli >= field->cap_stimuli) {
        field->cap_stimuli *= 2;
        field->stimuli = realloc(field->stimuli, sizeof(Stimulus) * field->cap_stimuli);

        if (!field->stimuli) {
            printf("[ERROR]: Array resize failed\n");
            exit(1);
        }
    }

    for (int i = 0; i < field->num_stimuli; ++i) {
        if (IN_BOUNDS(pos.x, field->stimuli[i].position.x - STATION_WIDTH, field->stimuli[i].position.x + STATION_WIDTH) && IN_BOUNDS(pos.y, field->stimuli[i].position.y - STATION_HEIGHT, field->stimuli[i].position.y + STATION_HEIGHT)) { 
            printf("[Warning]: \"Station to close to Station %d\" \n", i);
            return; 
        }
    }

    field->stimuli[field->num_stimuli].position = pos;
    field->stimuli[field->num_stimuli].strength = strength;

    field->num_stimuli++;
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