#pragma once
#include "GPU.hpp"
#include <volk.h>
#include <vector>
#include <unordered_map>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>

struct Swapchain {
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkFormat format;
    VkPresentModeKHR presentMode;
    VkExtent2D extent;
    std::vector<VkImage> images;
    std::vector<VkImageView> views;

    uint32_t recreateWidth,recreateHeight;
    bool needsRecreation;
};

class GPUVulkan final : public GPUDevice {
public:
    GPUVulkan(SDL_Window* window, bool lowPower, bool debug);
    ~GPUVulkan() override;
    
    bool RegisterWindow(SDL_Window* window) override;

private:
    bool RecreateSwapchain(SDL_Window* window);
    static bool OnResize(void *userdata, SDL_Event *event);

private:
    VkInstance m_instance;
    VkDevice m_logicalDevice;
    VkPhysicalDevice m_physicalDevice;
    VkDebugUtilsMessengerEXT m_debugMessenger;

    inline static std::unordered_map<SDL_Window*, Swapchain> m_swapchains;

    bool m_hasDebugUtils;
    bool m_hasColorSpaces;
};
