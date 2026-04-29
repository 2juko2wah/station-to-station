#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define WIDTH  400
#define HEIGHT 300

#define AGENT_CAPACITY 50000
#define STATION_CAPACITY 20

#define STATION_HEIGHT 16
#define STATION_WIDTH  16

#define DEPOSIT 20
#define DECAY 0.99
#define HEADING_SPEED 25
#define SENSOR_OFFSET 10
#define SENSOR_ANGLE  60

#define RANDOMNESS 0.2

typedef struct {
    float dt;
} State;

typedef struct {
    float x;
    float y;
} Vec2;

typedef struct {
    Vec2 position;

    Vec2 front_left;
    Vec2 front;
    Vec2 front_right;

    float heading; // normalized
} Agent;

typedef struct {
    uint8_t data[WIDTH][HEIGHT]; // [0, 1]

    uint8_t num_stations;
    uint8_t cap_stations;

    size_t num_agents;
    size_t cap_agents;

    Vec2 *stations;
    Agent *agents;
} Field;

static Field field;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

void InitField() {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            field.data[x][y] = 0;
        }
    }

    field.num_agents = AGENT_CAPACITY;
    field.num_stations = 0;

    field.cap_agents = AGENT_CAPACITY;
    field.cap_stations = STATION_CAPACITY;

    field.agents = SDL_malloc(sizeof(Agent) * AGENT_CAPACITY);
    field.stations = SDL_malloc(sizeof(Vec2) * STATION_CAPACITY);
    
}

#define IN_BOUNDS(x, a, b) x >= a && x <= b

void PlaceStation(size_t x, size_t y) {
    SDL_assert(field.stations);

    if (field.num_stations > field.cap_stations) {
        field.cap_stations *= 2;
        field.stations = SDL_realloc(field.stations, sizeof(Vec2) * field.cap_stations);

        if (!field.stations) {
            printf("[ERROR]: Array resize failed\n");
            exit(1);
        }
    }

    for (int i = 0; i < field.num_stations; ++i) {
        if (IN_BOUNDS(x, field.stations[i].x - STATION_WIDTH, field.stations[i].x + STATION_WIDTH) && IN_BOUNDS(y, field.stations[i].y - STATION_HEIGHT, field.stations[i].y + STATION_HEIGHT)) { 
            printf("[Warning]: \"Station to close to Station %d\" \n", i);
            return; 
        }
    }

    field.stations[field.num_stations++] = (Vec2) { .x = x, .y = y };
}

void RemoveStation(size_t x, size_t y) {
    for (int i = 0; i < field.num_stations; ++i) {
        if (IN_BOUNDS(x, field.stations[i].x - STATION_WIDTH / 2, field.stations[i].x + STATION_WIDTH / 2) && IN_BOUNDS(y, field.stations[i].y - STATION_HEIGHT / 2, field.stations[i].y + STATION_HEIGHT / 2)) { 
            field.stations[i] = field.stations[--field.num_stations];
            
        }
    }
}

Vec2 Vec2Add(Vec2 a, Vec2 b) {
    return (Vec2) { .x=a.x + b.x, .y=a.y + b.y };
}

Vec2 Vec2Scale(Vec2 a, float s) {
    return (Vec2) { .x=a.x*s, .y=a.y*s };
}

float Vec2Mag(Vec2 a) {
    return SDL_sqrtf(a.x*a.x + a.y*a.y);
}

Vec2 Vec2Norm(Vec2 a) {
    float mag = Vec2Mag(a);
    if (mag == 0.f) {
        return (Vec2) { .x = 0, .y = 0 };
    }

    return Vec2Scale(a, 1 / mag);
}

#define M_PI 3.14
Vec2 Vec2FromDeg(float deg) {
    return (Vec2) { .x = SDL_cos(deg * M_PI / 180.0), .y = SDL_sin(deg * M_PI / 180.0) };
}

float DegNorm(float deg) {
    return SDL_fmod(SDL_fmod(deg, 360) + 360, 360);
}

#define AGENT_SPEED 50

void InitAgents() {
    for (int i = 0; i < field.num_agents; ++i) {
        field.agents[i].heading = SDL_randf() * 360;
        field.agents[i].position = (Vec2) { .x = WIDTH / 2, .y = HEIGHT / 2 };

        field.agents[i].front = Vec2Add(field.agents[i].position, Vec2Scale(Vec2FromDeg(field.agents[i].heading), SENSOR_OFFSET));
        field.agents[i].front_left = Vec2Add(field.agents[i].position, Vec2Scale(Vec2FromDeg(field.agents[i].heading - SENSOR_ANGLE), SENSOR_OFFSET));
        field.agents[i].front_right = Vec2Add(field.agents[i].position, Vec2Scale(Vec2FromDeg(field.agents[i].heading + SENSOR_ANGLE), SENSOR_OFFSET));
    } 
}

void UpdateAgents(float dt) {
    for (int i = 0; i < field.num_agents; ++i) {
        if (field.agents[i].position.y < 0) {
            field.agents[i].position.y = 0;
            field.agents[i].heading = DegNorm(360.0f - field.agents[i].heading);
            
        } else if (field.agents[i].position.y >= HEIGHT) {
            field.agents[i].position.y = HEIGHT - 1;
            field.agents[i].heading = DegNorm(360.0f - field.agents[i].heading);
            
        }
        if (field.agents[i].position.x < 0) {
            field.agents[i].position.x = 0;
            field.agents[i].heading = DegNorm(180.0f - field.agents[i].heading);
            
        }  else if (field.agents[i].position.x >= WIDTH) {
            field.agents[i].position.x = (float)WIDTH - 1.0f;
            field.agents[i].heading = DegNorm(180.0f - field.agents[i].heading);
            
        }

        
        #define EDGE_MARGIN 2
        #define EDGE_TURN   10.0f

        if (field.agents[i].position.x < EDGE_MARGIN) {
            field.agents[i].heading = DegNorm(field.agents[i].heading + EDGE_TURN);
        } else if (field.agents[i].position.x > WIDTH - EDGE_MARGIN) {
            field.agents[i].heading = DegNorm(field.agents[i].heading - EDGE_TURN);
        }

        if (field.agents[i].position.y < EDGE_MARGIN) {
            field.agents[i].heading = DegNorm(field.agents[i].heading + EDGE_TURN);
        } else if (field.agents[i].position.y > HEIGHT - EDGE_MARGIN) {
            field.agents[i].heading = DegNorm(field.agents[i].heading - EDGE_TURN);
        }


        float left = 0;
        float forward = 0;
        float right = 0;
        
        if (!(field.agents[i].front_left.x >= WIDTH  || field.agents[i].front_left.x < 0 || field.agents[i].front_left.y >= HEIGHT || field.agents[i].front_left.y < 0)) {
            left = field.data[(int)field.agents[i].front_left.x][(int)field.agents[i].front_left.y];
        } 

        if (!(field.agents[i].front.x >= WIDTH  || field.agents[i].front.x < 0 || field.agents[i].front.y >= HEIGHT || field.agents[i].front.y < 0)) {
            forward = field.data[(int)field.agents[i].front.x][(int)field.agents[i].front.y];
        }
        
        if (!(field.agents[i].front_right.x >= WIDTH  || field.agents[i].front_right.x < 0 || field.agents[i].front_right.y >= HEIGHT || field.agents[i].front_right.y < 0)) {
            right = field.data[(int)field.agents[i].front_right.x][(int)field.agents[i].front_right.y];
        }

        if (left > forward && left > right) {
            field.agents[i].heading -= HEADING_SPEED;
        } else if (right > forward && right > left) {
            field.agents[i].heading += HEADING_SPEED;
        } else {
            field.agents[i].heading += (2 * (SDL_randf() - 0.5)) * RANDOMNESS;
        }

        field.data[(int)field.agents[i].position.x][(int)field.agents[i].position.y] = DEPOSIT;

        field.agents[i].position = Vec2Add(field.agents[i].position, Vec2Scale(Vec2FromDeg(field.agents[i].heading), AGENT_SPEED * dt));

        field.agents[i].front = Vec2Add(field.agents[i].position, Vec2Scale(Vec2FromDeg(field.agents[i].heading), SENSOR_OFFSET));
        field.agents[i].front_left = Vec2Add(field.agents[i].position,  Vec2Scale(Vec2FromDeg(DegNorm(field.agents[i].heading - SENSOR_ANGLE)), SENSOR_OFFSET));
        field.agents[i].front_right = Vec2Add(field.agents[i].position, Vec2Scale(Vec2FromDeg(DegNorm(field.agents[i].heading + SENSOR_ANGLE)), SENSOR_OFFSET));
    }
}

void UpdateField(float dt) {
    uint8_t buffer[WIDTH][HEIGHT];
    SDL_memcpy(buffer, field.data, sizeof(buffer));

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

    SDL_memcpy(field.data, buffer, (sizeof(buffer)));
}

void UpdateStations(float dt) {
    for (int i = 0; i < field.num_stations; ++i) {
        Vec2 pos = field.stations[i];
        for (int dy = -(STATION_HEIGHT / 2); dy <= (STATION_HEIGHT / 2); ++dy) {
            for (int dx = -(STATION_WIDTH / 2); dx <= (STATION_WIDTH / 2); ++dx) {
                if (pos.x + dx >= WIDTH || pos.x + dx < 0) continue;
                if (pos.y + dy >= HEIGHT || pos.y + dy < 0) continue;
                field.data[(int)(pos.x + dx)][(int)(pos.y + dy)] = 255;
            }
        }
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    *appstate = &(State) { .dt = 0.001 };
    
    SDL_SetAppMetadata("Slime Molds", "0.0", "com.jukowah.slime.molds");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Slime Molds", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer, WIDTH, HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    InitField();
    InitAgents();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    State *state = (State *)appstate;

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        SDL_MouseButtonEvent *mouse = (SDL_MouseButtonEvent *)event;
            
        float logical_x, logical_y;
        SDL_RenderCoordinatesFromWindow(renderer, mouse->x, mouse->y, &logical_x, &logical_y);
        
        switch (mouse->button) {
            case SDL_BUTTON_LEFT:
                PlaceStation((int)logical_x, (int)logical_y);
                break;
            case SDL_BUTTON_RIGHT:
                RemoveStation((int)logical_x, (int)logical_y);
                break;
        }
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        SDL_KeyboardEvent *keyboard = (SDL_KeyboardEvent *)event;
        switch (keyboard->key) {
            case SDLK_R:
                InitAgents();
                break;
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    State *state = (State *)appstate;

    static uint64_t last_time = 0;
    uint64_t now = SDL_GetTicks();
    
    if (last_time != 0) {
        state->dt = (now - last_time) / 1000.0f;
    }

    UpdateField(state->dt);
    UpdateStations(state->dt);
    UpdateAgents(state->dt);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            SDL_SetRenderDrawColor(renderer, 0, field.data[x][y], 0, 255);
            SDL_RenderPoint(renderer, x, y);
        }
    }

    for (int agent = 0; agent < field.num_agents; ++agent) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderPoint(renderer, field.agents[agent].position.x, field.agents[agent].position.y);
    }

    // for (int station = 0; station < field.num_stations; ++station) {
    //     SDL_SetRenderDrawColor(renderer, 123,123, 123, 255);
    //     SDL_RenderFillRect(renderer, &(SDL_FRect) { .x = field.stations[station].x - STATION_WIDTH / 2, .y = field.stations[station].y - STATION_HEIGHT / 2, .h = STATION_HEIGHT, .w = STATION_WIDTH });
    // }

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderPoint(renderer, field.agents[0].front.x, field.agents[0].front.y);
    SDL_RenderPoint(renderer, field.agents[0].front_left.x, field.agents[0].front_left.y);
    SDL_RenderPoint(renderer, field.agents[0].front_right.x, field.agents[0].front_right.y);

    

    SDL_RenderPresent(renderer);
    
    last_time = now;
    
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    return;
}
