#include "Engine/GPU.hpp"
#include "vulkan/VulkanDriver.hpp"

GPUDevice::GPUDevice(SDL_Window* window, bool lowPower, bool debug)
{
    m_pImpl = std::make_unique<Driver>(window,lowPower,debug);
}

GPUDevice::~GPUDevice()
{

}

void GPUDevice::Destroy() const
{
    m_pImpl->Destroy();
}

bool GPUDevice::RegisterWindow(SDL_Window* window) const 
{
    return m_pImpl->RegisterWindow(window);
}

void GPUDevice::UnregisterWindow(SDL_Window* window) const
{
    return m_pImpl->UnregisterWindow(window);
}

CommandBuffer* GPUDevice::AcquireCommandBuffer() const
{

}

bool GPUDevice::SubmitCommandBuffer(CommandBuffer* commandBuffer) const
{
    
}