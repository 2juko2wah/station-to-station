#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "defns.h"

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_gpu.h>

#define NUM_ELEMS 256
#define WORKGROUP_SIZE 64

struct AppState {
    SDL_Window   *window;
    SDL_GPUDevice *device;

    SDL_GPURenderPass *pass;

    SDL_GPUComputePipeline *computeline;

    SDL_GPUCommandBuffer *cmd;
    SDL_GPUTexture *swap;

    uint32_t swidth;
    uint32_t sheight;

    float dt;
    bool running;
};

static SDL_GPUComputePipeline *CreateComputePipelineFromShader(SDL_GPUDevice *device, const char *path, SDL_GPUComputePipelineCreateInfo *info) {
    size_t size;

    void *code = SDL_LoadFile(path, &size);

    if (!code) {
        SDL_Log("[ERROR/SDL3]: LoadFile failed: %s", SDL_GetError());

        return NULL;
    }

    SDL_GPUComputePipelineCreateInfo copy = *info;
    copy.code = (uint8_t *)code;
    copy.code_size = size;
    copy.entrypoint = "main";
    copy.format = SDL_GPU_SHADERFORMAT_SPIRV;

	SDL_GPUComputePipeline* pipeline = SDL_CreateGPUComputePipeline(device, &copy);
	if (!pipeline) {
		SDL_Log("[ERROR/SDL3]: CreateGPUComputePipeline: %f", SDL_GetError());
		SDL_free(code);
		return NULL;
	}
    
    SDL_free(code);

    return pipeline;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    AppState *state = (AppState *)SDL_malloc(sizeof(AppState));

    SDL_SetAppMetadata("Slime Molds", "0.0", "com.jukowah.slime.molds");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("[ERROR/SDL3]: SDL_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    state->window = SDL_CreateWindow("Slime Molds", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!state->window) {
        SDL_Log("[ERROR/SDL3]: CreateWindow failed: %s", SDL_GetError());
        
        SDL_free(state);

        return SDL_APP_FAILURE;
    }

    state->device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL, true, NULL);

    if (!state->device) {
        SDL_Log("[ERROR/SDL3]: CreateGPUDevice failed: %s", SDL_GetError());
        
        SDL_AppQuit(state, SDL_APP_FAILURE);
    }

    if (!SDL_ClaimWindowForGPUDevice(state->device, state->window)) {
        SDL_Log("[ERROR/SDL3]: ClaimWindowForGPUDevice failed: %s", SDL_GetError());

        SDL_AppQuit(state, SDL_APP_FAILURE);
    }
    SDL_RaiseWindow(state->window);

    SDL_GPUComputePipelineCreateInfo info;
    info.num_readonly_storage_textures = 1;
    info.num_readonly_storage_buffers = 1;
    info.num_uniform_buffers = 1;

    info.threadcount_x = WORKGROUP_SIZE;
    info.threadcount_y = 1;
    info.threadcount_z = 1;
    
    state->computeline = CreateComputePipelineFromShader(state->device, "test.spv", &info);

    state->running = true;

    *appstate = (void *)state;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    AppState *state = (AppState *)appstate;

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState *state = (AppState *)appstate;

    uint64_t last_time = 0;
    uint64_t curr_time = SDL_GetTicks();
    if (last_time != 0) state->dt = (curr_time - last_time) / 1000.0f;

    state->cmd = SDL_AcquireGPUCommandBuffer(state->device);
    if (!state->cmd) {
        SDL_Log("[ERROR/SDL3]: AcquireGPUCommandBuffer failed: %s", SDL_GetError());

        SDL_AppQuit(state, SDL_APP_FAILURE);
    }

    SDL_WaitAndAcquireGPUSwapchainTexture(state->cmd, state->window, &state->swap, &state->swidth, &state->sheight);

    if (!state->swap) {
        SDL_Log("[ERROR/SDL3]: WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());

        SDL_AppQuit(state, SDL_APP_FAILURE);
    }

    SDL_GPUColorTargetInfo target = {0};
    target.texture = state->swap;
    target.clear_color = (SDL_FColor) { 0.3f, 0.3f, 0.3f, 1.0f };
    target.load_op = SDL_GPU_LOADOP_CLEAR;
    target.store_op = SDL_GPU_STOREOP_STORE;

    state->pass = SDL_BeginGPURenderPass(state->cmd, &target, 1, NULL);

    SDL_EndGPURenderPass(state->pass);

    if (!SDL_SubmitGPUCommandBuffer(state->cmd)) {
        SDL_Log("[ERROR/SDL3]: SubmitGPUCommandBuffer failed: %s", SDL_GetError());

        SDL_AppQuit(state, SDL_APP_FAILURE);
    }

    last_time = curr_time;
    
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    AppState *state = (AppState *)appstate;

    if (state->window && state->device) SDL_ReleaseWindowFromGPUDevice(state->device, state->window);
    if (state->window) SDL_DestroyWindow(state->window);
    if (state->device) SDL_DestroyGPUDevice(state->device);

    SDL_free(state);
    return;
}
