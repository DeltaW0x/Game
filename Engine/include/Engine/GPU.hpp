#pragma once
#include <memory>
#include <SDL3/SDL_video.h>

typedef struct CommandBuffer CommandBuffer;

class GPUDevice {
public:
    GPUDevice(SDL_Window* window, bool lowPower, bool debug);
    ~GPUDevice();

    void Destroy() const;
    
    bool RegisterWindow(SDL_Window* window) const;
    void UnregisterWindow(SDL_Window* window) const;

    CommandBuffer* AcquireCommandBuffer() const;
    bool           SubmitCommandBuffer(CommandBuffer* commandBuffer) const;
    
private:
    class Driver;
    std::unique_ptr<Driver> m_pImpl;
};