project(Engine)

file(GLOB_RECURSE SOURCES
        LIST_DIRECTORIES true
        ./*.h
        ./*.hpp
        ./*.cpp)

add_compile_definitions(VK_NO_PROTOTYPES)
if(LINUX)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WAYLAND_KHR)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XCB_KHR)
endif()


add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3 spdlog::spdlog volk::volk vk-bootstrap::vk-bootstrap)


