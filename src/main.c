#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "vec2.h"
#include "defns.h"
#include "agent.h"

typedef struct {
    float dt;
} State;

static Field field;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

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

    InitField(&field);
    for (int i = 0; i < field.num_agents; ++i) {
        InitAgent(&(field.agents[i]));
    }

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
                PlaceStation(&field, (int)logical_x, (int)logical_y);
                break;
            case SDL_BUTTON_RIGHT:
                RemoveStation(&field, (int)logical_x, (int)logical_y);
                break;
        }
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        SDL_KeyboardEvent *keyboard = (SDL_KeyboardEvent *)event;
        switch (keyboard->key) {
            case SDLK_R:
                for (int i = 0; i < field.num_agents; ++i) {
                   InitAgent(&(field.agents[i]));
                }
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

    UpdateField(&field, state->dt);
    UpdateStations(&field, state->dt);
    
    for (int i = 0; i < field.num_agents; ++i) {
        UpdateAgent(&(field.agents[i]), &field, state->dt);
    }


    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            SDL_SetRenderDrawColor(renderer, 0, field.trail[x][y], 0, 255);
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
