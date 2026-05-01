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

typedef struct PositionVertex
{
	float x, y, z;
} PositionVertex;

typedef struct PositionColorVertex
{
	float x, y, z;
	Uint8 r, g, b, a;
} PositionColorVertex;

typedef struct PositionTextureVertex
{
    float x, y, z;
    float u, v;
} PositionTextureVertex;


typedef struct AppState {
    SDL_Window   *window;
    SDL_GPUDevice *device;

    SDL_GPURenderPass *pass;

    SDL_GPUComputePipeline *computeline;
    SDL_GPUGraphicsPipeline *drawline;

    SDL_GPUCommandBuffer *cmd;
    SDL_GPUTexture *swap;

    SDL_GPUTexture *texture;
    SDL_GPUSampler *sampler;

    SDL_GPUBuffer* vbuff;


    uint32_t swidth;
    uint32_t sheight;

    int32_t width;
    int32_t height;

    float dt;
    bool running;
} AppState;

SDL_GPUShader* LoadShader(SDL_GPUDevice* device, const char *path, Uint32 samplerCount, Uint32 uniformBufferCount, Uint32 storageBufferCount, Uint32 storageTextureCount) {
	// Auto-detect the shader stage from the file name for convenience
	SDL_GPUShaderStage stage;
	if (SDL_strstr(path, "vert")) {
		stage = SDL_GPU_SHADERSTAGE_VERTEX;
	}
	else if (SDL_strstr(path, "frag")) {
		stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
	}
	else {
		SDL_Log("Invalid shader stage!");
		return NULL;
	}

	size_t size;
	void* code = SDL_LoadFile(path, &size);
	if (!code) {
		SDL_Log("[ERROR/SDL3]: LoadFile failed: %s", path);
		return NULL;
	}

	SDL_GPUShaderCreateInfo shaderInfo;
	shaderInfo.code = (uint8_t *)code;
	shaderInfo.code_size = size;
	shaderInfo.entrypoint = "main";
	shaderInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
	shaderInfo.stage = stage;
	shaderInfo.num_samplers = samplerCount;
	shaderInfo.num_uniform_buffers = uniformBufferCount;
	shaderInfo.num_storage_buffers = storageBufferCount;
	shaderInfo.num_storage_textures = storageTextureCount;
	
	SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
	if (!shader) {
		SDL_Log("[ERROR/SDL3]: CreateGPUShader failed: %s", SDL_GetError());
		SDL_free(code);
		return NULL;
	}

	SDL_free(code);
	return shader;
}

static SDL_GPUComputePipeline *CreateComputePipelineFromShader(SDL_GPUDevice *device, const char *path, const SDL_GPUComputePipelineCreateInfo info) {
    size_t size;

    void *code = SDL_LoadFile(path, &size);

    if (!code) {
        SDL_Log("[ERROR/SDL3]: LoadFile failed: %s", SDL_GetError());

        return NULL;
    }

    SDL_GPUComputePipelineCreateInfo copy = info;
    copy.code = (uint8_t *)code;
    copy.code_size = size;
    copy.entrypoint = "main";
    copy.format = SDL_GPU_SHADERFORMAT_SPIRV;

	SDL_GPUComputePipeline* pipeline = SDL_CreateGPUComputePipeline(device, &copy);
	if (!pipeline) {
		SDL_Log("[ERROR/SDL3]: CreateGPUComputePipeline: %s", SDL_GetError());
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

    SDL_GetWindowSizeInPixels(state->window, &state->width, &state->height);
    SDL_RaiseWindow(state->window);

    SDL_GPUShader *vertexShader = LoadShader(state->device, "vert.spv", 0, 0, 0, 0);
    if (!vertexShader) {
        SDL_Log("Failed to create fragment shader!");
        SDL_AppQuit(state, SDL_APP_FAILURE);
    }

    SDL_GPUShader *fragmentShader = LoadShader(state->device, "frag.spv", 1, 0, 0, 0);
    if (!fragmentShader) {
        SDL_Log("Failed to create fragment shader!");
        SDL_AppQuit(state, SDL_APP_FAILURE);
    }


    SDL_GPUComputePipelineCreateInfo cinfo = {0};

    cinfo.num_readwrite_storage_textures = 1;

    cinfo.threadcount_x = 8;
    cinfo.threadcount_y = 8;
    cinfo.threadcount_z = 1;
    
    state->computeline = CreateComputePipelineFromShader(state->device, "comp.spv", cinfo);

    SDL_GPUVertexBufferDescription vertBuffDescs[] = {{
    .slot = 0,
    .pitch = sizeof(PositionTextureVertex),
    .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
    .instance_step_rate = 0,
}};

SDL_GPUVertexAttribute vertAttribs[] = {{
    .location = 0,
    .buffer_slot = 0,
    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
    .offset = 0
}, {
    .location = 1,
    .buffer_slot = 0,
    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
    .offset = sizeof(float) * 3
}};

SDL_GPUColorTargetDescription desc = {
    .format = SDL_GetGPUSwapchainTextureFormat(state->device, state->window)
};

SDL_GPUGraphicsPipelineCreateInfo pinfo = {
    .vertex_shader = vertexShader,
    .fragment_shader = fragmentShader,
    .vertex_input_state = {
        .vertex_buffer_descriptions = vertBuffDescs,
        .num_vertex_buffers = 1,
        .vertex_attributes = vertAttribs,
        .num_vertex_attributes = 2,
    },
    .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
    .target_info = {
        .color_target_descriptions = &desc,
        .num_color_targets = 1,
    },
};

state->drawline = SDL_CreateGPUGraphicsPipeline(state->device, &pinfo);

    SDL_ReleaseGPUShader(state->device, vertexShader);
    SDL_ReleaseGPUShader(state->device, fragmentShader);

    SDL_GPUTextureCreateInfo tinfo;
    tinfo.type = SDL_GPU_TEXTURETYPE_2D;
    tinfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    tinfo.width = state->width;
    tinfo.height = state->height;
    tinfo.layer_count_or_depth = 1;
    tinfo.num_levels = 1;
    tinfo.usage = SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE | SDL_GPU_TEXTUREUSAGE_SAMPLER;
    state->texture = SDL_CreateGPUTexture(state->device, &tinfo);

    SDL_GPUSamplerCreateInfo sinfo = {0};
    sinfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
    sinfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
    state->sampler = SDL_CreateGPUSampler(state->device, &sinfo);

    state->running = true;
    *appstate = (void *)state;


    SDL_GPUBufferCreateInfo binfo= {0};
    binfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    binfo.size = sizeof(PositionTextureVertex) * 6;
    
    state->vbuff = SDL_CreateGPUBuffer(
        state->device,
        &binfo    
    );

    SDL_GPUTransferBufferCreateInfo bcinfo = {0};
    bcinfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    bcinfo.size = sizeof(PositionTextureVertex) * 6;
        
    SDL_GPUTransferBuffer *transferBuffer = SDL_CreateGPUTransferBuffer(
		state->device,
        &bcinfo
	);

	PositionTextureVertex *transferData = (PositionTextureVertex *)SDL_MapGPUTransferBuffer(
		state->device,
		transferBuffer,
		false
	);

    transferData[0] = (PositionTextureVertex) { -1, -1, 0, 0, 0 };
	transferData[1] = (PositionTextureVertex) {  1, -1, 0, 1, 0 };
	transferData[2] = (PositionTextureVertex) {  1,  1, 0, 1, 1 };
	transferData[3] = (PositionTextureVertex) { -1, -1, 0, 0, 0 };
	transferData[4] = (PositionTextureVertex) {  1,  1, 0, 1, 1 };
	transferData[5] = (PositionTextureVertex) { -1,  1, 0, 0, 1 };

	SDL_UnmapGPUTransferBuffer(state->device, transferBuffer);

	state->cmd = SDL_AcquireGPUCommandBuffer(state->device);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(state->cmd);


    SDL_GPUTransferBufferLocation bloc = {0}; 
	bloc.transfer_buffer = transferBuffer;
	bloc.offset = 0;
		
    SDL_GPUBufferRegion breg;
	breg.buffer = state->vbuff;
	breg.offset = 0;
	breg.size = sizeof(PositionTextureVertex) * 6;
	
	SDL_UploadToGPUBuffer(
		copyPass,
		&bloc,
		&breg,
		false
	);

	SDL_EndGPUCopyPass(copyPass);

    SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(
        state->cmd,
        (SDL_GPUStorageTextureReadWriteBinding[]){{
            .texture = state->texture
        }},
        1,
        NULL,
        0
    );

    SDL_BindGPUComputePipeline(computePass, state->computeline);
    SDL_DispatchGPUCompute(computePass, state->width / 8, state->height / 8, 1);
    SDL_EndGPUComputePass(computePass);

	SDL_SubmitGPUCommandBuffer(state->cmd);

    SDL_ReleaseGPUComputePipeline(state->device, state->computeline);
	SDL_ReleaseGPUTransferBuffer(state->device, transferBuffer);

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

    if (state->swap && state->cmd) {
        SDL_GPUColorTargetInfo targets[1] = {0};
        targets[0].texture = state->swap;
        targets[0].clear_color.a = 1;
        targets[0].load_op = SDL_GPU_LOADOP_CLEAR;
        targets[0].store_op = SDL_GPU_STOREOP_STORE;
        targets[0].cycle = false;

        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(
            state->cmd,
            targets,
            1,
            NULL
        );

        SDL_BindGPUGraphicsPipeline(renderPass, state->drawline);
        SDL_GPUBufferBinding binding; 
        binding.buffer = state->vbuff; 
        binding.offset = 0;
        SDL_BindGPUVertexBuffers(renderPass, 0, &binding, 1);
        SDL_GPUTextureSamplerBinding samplerbinding; 
        samplerbinding.texture = state->texture;
        samplerbinding.sampler = state->sampler; 
        SDL_BindGPUFragmentSamplers(renderPass, 0, &samplerbinding, 1);
        SDL_DrawGPUPrimitives(renderPass, 6, 1, 0, 0);

        SDL_EndGPURenderPass(renderPass);
    
    }
    SDL_SubmitGPUCommandBuffer(state->cmd);
    
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    AppState *state = (AppState *)appstate;
    
    if (state->device) {
        if (state->drawline)  SDL_ReleaseGPUGraphicsPipeline(state->device, state->drawline);
        if (state->texture)   SDL_ReleaseGPUTexture(state->device, state->texture);
        if (state->sampler)   SDL_ReleaseGPUSampler(state->device, state->sampler);
        if (state->vbuff)     SDL_ReleaseGPUBuffer(state->device, state->vbuff);

        if (state->window) SDL_ReleaseWindowFromGPUDevice(state->device, state->window);
        SDL_DestroyGPUDevice(state->device);
    }

    if (state->window) SDL_DestroyWindow(state->window);



    SDL_free(state);
    return;
}
