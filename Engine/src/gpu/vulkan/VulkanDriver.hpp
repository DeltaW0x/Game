#pragma once
#include "Engine/GPU.hpp"
#include <volk.h>
#include <deque>
#include <vector>
#include <unordered_map>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>

struct Swapchain {
    SDL_Window* window;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkFormat format;
    VkPresentModeKHR presentMode;
    VkExtent2D extent;
    std::vector<VkImage> images;
    std::vector<VkImageView> views;
    Uint32 swapchainImageIndex;
    uint32_t recreateWidth,recreateHeight;
    bool needsRecreation;
};

typedef struct VulkanCommandPool VulkanCommandPool;

struct VulkanCommandBuffer {
    VkCommandBuffer commandBuffer;
    VulkanCommandPool* commandPool;
    std::vector<Swapchain*> swapchains;
    std::vector<VkSemaphore> waitSemaphore;
    std::vector<VkSemaphore> signalSemaphore;
    std::vector<uint64_t> semaphoreValues;
};

struct VulkanCommandPool {
    SDL_ThreadID threadID;
    VkCommandPool commandPool;
    std::deque<VulkanCommandBuffer*> inactiveCommandBuffers;
};

class GPUDevice::Driver {
public:
    Driver(SDL_Window* window, bool lowPower, bool debug);
    ~Driver() = default;
    
    void Destroy();
    
    bool RegisterWindow(SDL_Window* window);
    void UnregisterWindow(SDL_Window* window);
    
    CommandBuffer* AcquireCommandBuffer();
    bool SubmitCommandBuffer(CommandBuffer* commandBuffer);

private:
    bool RecreateSwapchain(SDL_Window* window);
    VulkanCommandBuffer* FetchCommandBuffer(SDL_ThreadID threadID);
    VulkanCommandPool* FetchCommandPool(SDL_ThreadID threadID);

    static bool OnResize(void *userdata, SDL_Event *event);

    VkInstance m_instance;
    VkDevice m_logicalDevice;
    VkPhysicalDevice m_physicalDevice;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    
    VkQueue m_queue;
    uint32_t m_queueIndex;

    SDL_Mutex* m_commandBufferLock;
    std::unordered_map<SDL_ThreadID, VulkanCommandPool> m_commandPools;

    inline static std::unordered_map<SDL_Window*, Swapchain> m_swapchains;
    bool m_hasDebugUtils;
    bool m_hasColorSpaces;
};
