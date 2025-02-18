#include <Engine/GPU.hpp>
#include <Engine/GPUVulkan.hpp>
#include <SDL3/SDL.h>
int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("HEAP",1280,720,SDL_WINDOW_VULKAN);
    GPUDevice* device = new GPUVulkan(window,false,true);
    delete device;
}