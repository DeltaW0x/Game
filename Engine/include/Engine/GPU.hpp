#pragma once
#include <SDL3/SDL_video.h>

class GPUDevice {
public:
    GPUDevice() {};
    virtual ~GPUDevice() = default;

    virtual bool RegisterWindow(SDL_Window* window) = 0;
};