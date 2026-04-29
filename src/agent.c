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

    agent->front       = Vec2Add(agent->position, Vec2Scale(Vec2FromDeg(agent->heading), SENSOR_OFFSET));
    agent->front_left  = Vec2Add(agent->position, Vec2Scale(Vec2FromDeg(agent->heading - SENSOR_ANGLE), SENSOR_OFFSET));
    agent->front_right = Vec2Add(agent->position, Vec2Scale(Vec2FromDeg(agent->heading + SENSOR_ANGLE), SENSOR_OFFSET));
}

void UpdateAgent(Agent *agent, Field *field, float dt) {
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

        float left = 0.0f;
        float forward = 0.0f;
        float right = 0.0f;
        
        if (!(agent->front_left.x >= WIDTH  || agent->front_left.x < 0.0f || agent->front_left.y >= HEIGHT || agent->front_left.y < 0.0f)) {
            left = field->trail[(int)agent->front_left.x][(int)agent->front_left.y];
        } 

        if (!(agent->front.x >= WIDTH  || agent->front.x < 0.0f || agent->front.y >= HEIGHT || agent->front.y < 0.0f)) {
            forward = field->trail[(int)agent->front.x][(int)agent->front.y];
        }
        
        if (!(agent->front_right.x >= WIDTH  || agent->front_right.x < 0.0f || agent->front_right.y >= HEIGHT || agent->front_right.y < 0.0f)) {
            right = field->trail[(int)agent->front_right.x][(int)agent->front_right.y];
        }

        if (left > forward && left > right) {
            agent->heading -= HEADING_SPEED;
        } else if (right > forward && right > left) {
            agent->heading += HEADING_SPEED;
        } else {
            agent->heading += (2.0f * (SDL_randf() - 0.5f)) * RANDOMNESS;
        }

        field->trail[(int)agent->position.x][(int)agent->position.y] = DEPOSIT;

        agent->position = Vec2Add(agent->position, Vec2Scale(Vec2FromDeg(agent->heading), AGENT_SPEED * dt));

        agent->front       = Vec2Add(agent->position, Vec2Scale(Vec2FromDeg(agent->heading), SENSOR_OFFSET));
        agent->front_left  = Vec2Add(agent->position, Vec2Scale(Vec2FromDeg(DegNorm(agent->heading - SENSOR_ANGLE)), SENSOR_OFFSET));
        agent->front_right = Vec2Add(agent->position, Vec2Scale(Vec2FromDeg(DegNorm(agent->heading + SENSOR_ANGLE)), SENSOR_OFFSET));
}

void InitField(Field *field) {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            field->trail[x][y] = 0;
        }
    }

    field->num_agents   = AGENT_CAPACITY;
    field->num_stations = 0;

    field->cap_agents   = AGENT_CAPACITY;
    field->cap_stations = STATION_CAPACITY;

    field->agents   = SDL_malloc(sizeof(Agent) * AGENT_CAPACITY);
    field->stations = SDL_malloc(sizeof(Vec2) * STATION_CAPACITY);
}

void UpdateField(Field *field, float dt) {
    uint8_t buffer[WIDTH][HEIGHT];
    
    SDL_memcpy(buffer, field->trail, sizeof(buffer));

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            uint8_t num_edges = 4;

            if (x+1 >= WIDTH) num_edges--;
            else if (x-1 < 0) num_edges--;
            if (y+1 >= HEIGHT) num_edges--;
            else if (y-1 < 0) num_edges--;

            buffer[x][y] = SDL_clamp(buffer[x][y], 0, 255);
            buffer[x][y] = (buffer[x][y-1]*(y-1 >= 0) + buffer[x-1][y]*(x-1 >= 0) + buffer[x][y] + buffer[x+1][y]*(x+1 < WIDTH) + buffer[x][y+1]*(y+1 < HEIGHT)) / (num_edges + 1);

            buffer[x][y] *= DECAY;
        }
    }

    SDL_memcpy(field->trail, buffer, (sizeof(buffer)));
}


void PlaceStation(Field *field, uint64_t x, uint64_t y) {
    if (field->num_stations > field->cap_stations) {
        field->cap_stations *= 2;
        field->stations = realloc(field->stations, sizeof(Vec2) * field->cap_stations);

        if (!field->stations) {
            printf("[ERROR]: Array resize failed\n");
            exit(1);
        }
    }

    for (int i = 0; i < field->num_stations; ++i) {
        if (IN_BOUNDS(x, field->stations[i].x - STATION_WIDTH, field->stations[i].x + STATION_WIDTH) && IN_BOUNDS(y, field->stations[i].y - STATION_HEIGHT, field->stations[i].y + STATION_HEIGHT)) { 
            printf("[Warning]: \"Station to close to Station %d\" \n", i);
            return; 
        }
    }

    field->stations[field->num_stations++] = (Vec2) { .x = x, .y = y };
}

void RemoveStation(Field *field, uint64_t x, uint64_t y) {
    for (int i = 0; i < field->num_stations; ++i) {
        if (IN_BOUNDS(x, field->stations[i].x - STATION_WIDTH / 2, field->stations[i].x + STATION_WIDTH / 2) && IN_BOUNDS(y, field->stations[i].y - STATION_HEIGHT / 2, field->stations[i].y + STATION_HEIGHT / 2)) { 
            field->stations[i] = field->stations[--field->num_stations];
            
        }
    }
}

void UpdateStations(Field *field, float dt) {
    for (int i = 0; i < field->num_stations; ++i) {
        Vec2 pos = field->stations[i];
        for (int dy = -(STATION_HEIGHT / 2.0f); dy <= (STATION_HEIGHT / 2.0f); ++dy) {
            for (int dx = -(STATION_WIDTH / 2.0f); dx <= (STATION_WIDTH / 2.0f); ++dx) {
                if (pos.x + dx >= WIDTH || pos.x + dx < 0) continue;
                if (pos.y + dy >= HEIGHT || pos.y + dy < 0) continue;
                field->trail[(int)(pos.x + dx)][(int)(pos.y + dy)] = 255;
            }
        }
    }
}