#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#include "imgui.h"

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include "vec2.h"
#include "defns.h"
#include "agent.h"

typedef struct {
    float dt;
    ImGuiIO *io;
} State;

static Field field;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static SDL_Texture *trail_texture = NULL;
static uint8_t pbuffer[HEIGHT][WIDTH][4];

bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    State *state = (State *)SDL_malloc(sizeof(State));

    state->dt = 0.001f;
    *appstate = state;
    
    SDL_SetAppMetadata("Slime Molds", "0.0", "com.jukowah.slime.molds");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

    if (!SDL_CreateWindowAndRenderer("Slime Molds", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

        IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    state->io = &ImGui::GetIO();
    state->io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    state->io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    trail_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    SDL_SetRenderLogicalPresentation(renderer, WIDTH, HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    InitField(&field);
    for (int i = 0; i < AGENT_CAPACITY; ++i) {
        InitAgent(&(field.agents[i]));
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    State *state = (State *)appstate;

    ImGui_ImplSDL3_ProcessEvent(event);

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        SDL_MouseButtonEvent *mouse = (SDL_MouseButtonEvent *)event;
            
        float logical_x, logical_y;
        SDL_RenderCoordinatesFromWindow(renderer, mouse->x, mouse->y, &logical_x, &logical_y);
        
        switch (mouse->button) {
            case SDL_BUTTON_LEFT:
                PlaceStimulus(&field, Vec2New((int)logical_x, (int)logical_y), 255);
                break;
            case SDL_BUTTON_MIDDLE:
                RemoveStimulus(&field, Vec2New((int)logical_x, (int)logical_y));
                RemoveStimulus(&field, Vec2New((int)logical_x, (int)logical_y));
                break;
            case SDL_BUTTON_RIGHT:
                PlaceStimulus(&field, Vec2New((int)logical_x, (int)logical_y), -255);
                break;
        }
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        SDL_KeyboardEvent *keyboard = (SDL_KeyboardEvent *)event;
        switch (keyboard->key) {
            case SDLK_R:
                for (int i = 0; i < AGENT_CAPACITY; ++i) {
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
    UpdateStimuli(&field, state->dt);
    
    for (int i = 0; i < AGENT_CAPACITY; ++i) {
        UpdateAgent(&(field.agents[i]), &field, state->dt);
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            pbuffer[y][x][0] = 255;
            pbuffer[y][x][1] = 0;
            pbuffer[y][x][2] = 0;
            pbuffer[y][x][3] = 0;
        }
    }


    for (int agent = 0; agent < AGENT_CAPACITY; ++agent) {
        Vec2 p = field.agents[agent].position;

        if (p.x >= 0 && p.x < WIDTH && p.y >= 0 && p.y < HEIGHT) {
            pbuffer[(int)p.y][(int)p.x][0] = 255;
            pbuffer[(int)p.y][(int)p.x][1] = 255;
            pbuffer[(int)p.y][(int)p.x][2] = 255;
            pbuffer[(int)p.y][(int)p.x][3] = 255;
        }
    }


    for (int agent = 0; agent < field.num_stimuli; ++agent) {
        Vec2 pos = field.stimuli[agent].position;
        scalar str = field.stimuli[agent].strength;

        for (int dy = -(STATION_HEIGHT / 2.0f); dy <= (STATION_HEIGHT / 2.0f); ++dy) {
            for (int dx = -(STATION_WIDTH / 2.0f); dx <= (STATION_WIDTH / 2.0f); ++dx) {
                if (pos.x + dx >= 0 && pos.x + dx < WIDTH && pos.y + dy >= 0 && pos.y + dy < HEIGHT) {
                    pbuffer[(int)pos.y+dy][(int)pos.x+dx][0] = 255;
                    pbuffer[(int)pos.y+dy][(int)pos.x+dx][1] = 0;
                    pbuffer[(int)pos.y+dy][(int)pos.x+dx][2] = (str > 0) * 255;
                    pbuffer[(int)pos.y+dy][(int)pos.x+dx][3] = (str < 0) * 255;
                }
            }
        }

    }

    SDL_UpdateTexture(trail_texture, NULL, pbuffer, WIDTH*4);


    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }
    
    ImGui::Render();
    
    SDL_SetRenderScale(renderer, state->io->DisplayFramebufferScale.x, state->io->DisplayFramebufferScale.y);
    SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    SDL_RenderClear(renderer);

    SDL_RenderTexture(renderer,trail_texture, NULL, NULL);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

    SDL_RenderPresent(renderer);

    last_time = now;
    
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    SDL_DestroyTexture(trail_texture);
    SDL_free(appstate);

    return;
}
