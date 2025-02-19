#include <Engine/GPU.hpp>
#include <spdlog/spdlog.h>
#include <Engine/GPU.hpp>
#include <SDL3/SDL.h>

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Paw", 1280, 720, SDL_WINDOW_VULKAN);
    GPUDevice device(window,true,true);
    device.Destroy();
    SDL_DestroyWindow(window);
    SDL_Quit();
}
