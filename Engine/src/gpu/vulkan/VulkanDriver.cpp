#include "VulkanDriver.hpp"
#include <VkBootstrap.h>
#include <spdlog/spdlog.h>
#include <SDL3/SDL_vulkan.h>

#define VK_CHECK_ABORT(value, message) if(value != VK_SUCCESS) {spdlog::critical(message); std::abort();}

GPUDevice::Driver::Driver(SDL_Window* window, bool lowPower, bool debug)
{
    VK_CHECK_ABORT(volkInitialize(), "Failed to initialize Volk")
    auto systemInfoRes = vkb::SystemInfo::get_system_info();
    m_hasDebugUtils = systemInfoRes.has_value() && systemInfoRes.value().is_extension_available(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    m_hasColorSpaces = systemInfoRes.has_value() && systemInfoRes.value().is_extension_available(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);

    vkb::InstanceBuilder instanceBuilder;
    instanceBuilder
        .set_engine_name("Paw")
        .request_validation_layers(debug)
        .use_default_debug_messenger()
        .require_api_version(VK_API_VERSION_1_2);
    if (m_hasDebugUtils) {
        instanceBuilder.enable_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    if (m_hasColorSpaces) {
        instanceBuilder.enable_extension(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
    }

    auto instanceRes = instanceBuilder.build();
    if (!instanceRes.has_value()) {
        spdlog::critical("Failed to create Vulkan instance, {0}",instanceRes.error().message());
        std::abort();
    }

    m_instance = instanceRes.value().instance;
    m_debugMessenger = instanceRes.value().debug_messenger;
    m_commandBufferLock = SDL_CreateMutex();

    volkLoadInstance(m_instance);

    Swapchain swapchain;
    swapchain.swapchain = VK_NULL_HANDLE;
    swapchain.window = window;

    if (!SDL_Vulkan_CreateSurface(window,m_instance,nullptr,&swapchain.surface)) {
        spdlog::critical("Failed to create Vulkan surface, {0}",SDL_GetError());
        std::abort();
    }

    VkPhysicalDeviceFeatures features = {
        .imageCubeArray = VK_TRUE,
        .multiDrawIndirect = VK_TRUE,
        .multiViewport = VK_TRUE,
        .samplerAnisotropy = VK_TRUE,
        .textureCompressionBC = VK_TRUE
    };
    VkPhysicalDeviceVulkan11Features features11 = {
        .multiview = VK_TRUE,
    };
    VkPhysicalDeviceVulkan12Features features12 = {
        .descriptorIndexing = VK_TRUE,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .shaderStorageImageArrayNonUniformIndexing = VK_TRUE,
        .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingUpdateUnusedWhilePending = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .timelineSemaphore = VK_TRUE,
        .bufferDeviceAddress = VK_TRUE
    };
    VkPhysicalDeviceSynchronization2FeaturesKHR syncronization2Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR,
        .synchronization2 = VK_TRUE
    };
    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
        .dynamicRendering = VK_TRUE
    };

    vkb::PhysicalDeviceSelector physicalDeviceSelector(instanceRes.value());
    auto physDeviceRes = physicalDeviceSelector
        .require_present(true)
        .set_surface(swapchain.surface)
        .set_required_features(features)
        .set_required_features_11(features11)
        .set_required_features_12(features12)
        .add_required_extension_features(syncronization2Features)
        .add_required_extension_features(dynamicRenderingFeatures)
        .add_required_extensions({VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME})
        .prefer_gpu_device_type(lowPower ? vkb::PreferredDeviceType::integrated : vkb::PreferredDeviceType::discrete)
        .select();

    if (!physDeviceRes.has_value()) {
        spdlog::critical("Failed to create find a suitable Vulkan device, {0}",physDeviceRes.error().message());
        std::abort();
    }
    vkb::Device vkbDevice = vkb::DeviceBuilder(physDeviceRes.value()).build().value();
    m_logicalDevice = vkbDevice.device;
    m_physicalDevice = vkbDevice.physical_device;

    SDL_SyncWindow(window);
    if (!SDL_GetWindowSizeInPixels(window,reinterpret_cast<int*>(&swapchain.recreateWidth),reinterpret_cast<int*>(&swapchain.recreateHeight))){
        spdlog::critical("Failed to get main window size, {0}",SDL_GetError());
        std::abort();
    }
    m_swapchains.emplace(window,swapchain);
    SDL_AddEventWatch(OnResize,(void*)window);
    if (!RecreateSwapchain(window)) {
        std::abort();
    }
    m_queue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    m_queueIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    spdlog::info("Vulkan device [{0}] created successfully",vkbDevice.physical_device.name);
}

void GPUDevice::Driver::Destroy() 
{
    vkDeviceWaitIdle(m_logicalDevice);
    for(auto& keyval : m_swapchains){
       UnregisterWindow(keyval.first);
    }
    vkDestroyDevice(m_logicalDevice,nullptr);
    vkDestroyDebugUtilsMessengerEXT(m_instance,m_debugMessenger,nullptr);
    vkDestroyInstance(m_instance,nullptr);
}

bool GPUDevice::Driver::RegisterWindow(SDL_Window *window)
{
    Swapchain swapchain;
    swapchain.swapchain = VK_NULL_HANDLE;
    swapchain.window = window;

    if (!SDL_Vulkan_CreateSurface(window,m_instance,nullptr,&swapchain.surface)) {
        spdlog::error("Failed to create Vulkan surface, {0}",SDL_GetError());
        return false;
    }

    SDL_SyncWindow(window);
    if (!SDL_GetWindowSizeInPixels(window,reinterpret_cast<int*>(&swapchain.recreateWidth),reinterpret_cast<int*>(&swapchain.recreateHeight))){
        spdlog::error("Failed to get window size, {0}",SDL_GetError());
        return false;
    }
    m_swapchains.emplace(window,swapchain);
    SDL_AddEventWatch(OnResize,(void*)window);
    return RecreateSwapchain(window);
}

void GPUDevice::Driver::UnregisterWindow(SDL_Window* window) 
{
    Swapchain& swapchain = m_swapchains.at(window); 
    for(auto& view : swapchain.views){
        vkDestroyImageView(m_logicalDevice,view,nullptr);
    }
    vkDestroySwapchainKHR(m_logicalDevice,swapchain.swapchain,nullptr);
    vkDestroySurfaceKHR(m_instance,swapchain.surface,nullptr);
}

CommandBuffer* GPUDevice::Driver::AcquireCommandBuffer()
{
    SDL_ThreadID threadID = SDL_GetCurrentThreadID();
    SDL_LockMutex(m_commandBufferLock);
}

bool GPUDevice::Driver::SubmitCommandBuffer(CommandBuffer* commandBuffer)
{
    
}

bool GPUDevice::Driver::RecreateSwapchain(SDL_Window *window)
{
    Swapchain& swapchain = m_swapchains.at(window);
    vkb::SwapchainBuilder swapchainBuilder(m_physicalDevice,m_logicalDevice,swapchain.surface);
    auto swapchainRes = swapchainBuilder
        .set_desired_format(VkSurfaceFormatKHR{ .format = VK_FORMAT_B8G8R8A8_UNORM, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_extent(swapchain.recreateWidth, swapchain.recreateHeight)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
        .set_old_swapchain(swapchain.swapchain)
        .set_desired_min_image_count(2)
        .build();
    if (!swapchainRes.has_value()) {
        spdlog::critical("Failed to create swapchain, {0}",swapchainRes.error().message());
        return false;
    }
    for (auto& view : swapchain.views) {
        vkDestroyImageView(m_logicalDevice,view,nullptr);
    }
    vkDestroySwapchainKHR(m_logicalDevice,swapchain.swapchain,nullptr);

    vkb::Swapchain vkbSwapchain = swapchainRes.value();
    swapchain.swapchain = vkbSwapchain.swapchain;
    swapchain.format = vkbSwapchain.image_format;
    swapchain.presentMode = vkbSwapchain.present_mode;
    swapchain.extent = vkbSwapchain.extent;
    swapchain.images = vkbSwapchain.get_images().value();
    swapchain.views = vkbSwapchain.get_image_views().value();
    swapchain.needsRecreation = false;
    return true;
}

VulkanCommandBuffer* GPUDevice::Driver::FetchCommandBuffer(SDL_ThreadID threadID) 
{
  return nullptr;
}

VulkanCommandPool* GPUDevice::Driver::FetchCommandPool(SDL_ThreadID threadID) 
{
    if(m_commandPools.contains(threadID)){
        return &m_commandPools.at(threadID);
    }

    VulkanCommandPool commandPool = {
        .threadID = threadID,
        .inactiveCommandBuffers = {}
    };

    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = m_queueIndex
    };
    VK_CHECK_ABORT(vkCreateCommandPool(m_logicalDevice,&commandPoolCreateInfo,NULL,&commandPool.commandPool), "Failed to create vulkan command pool");
    m_commandPools.emplace(threadID,commandPool);
    return &m_commandPools.at(threadID);
}

bool GPUDevice::Driver::OnResize(void *userdata, SDL_Event *event) {
  if (event->type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED &&
      event->window.windowID == SDL_GetWindowID((SDL_Window *)userdata)) {
    Swapchain &swapchain = m_swapchains[(SDL_Window *)userdata];
    swapchain.recreateWidth = event->window.data1;
    swapchain.recreateHeight = event->window.data2;
    swapchain.needsRecreation = true;
  }
  return true;
}