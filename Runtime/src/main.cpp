#include <Engine/GPU.hpp>
#include <spdlog/spdlog.h>
#include <Engine/GPUVulkan.hpp>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL.h>

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    //This is stupid as hell but works on MacOS
    volkInitialize();
    SDL_Window* window = SDL_CreateWindow("Paw", 1280, 720, SDL_WINDOW_VULKAN);
    volkFinalize();
    
    GPUDevice* device = new GPUVulkan(window,true,true);
    SDL_Quit();
}
