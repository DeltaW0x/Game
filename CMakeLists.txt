cmake_minimum_required (VERSION 3.10)
project("SuperProject")
set(CMAKE_CXX_STANDARD 20)

include(CPM.cmake)
find_program(CCACHE_FOUND ccache)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CCACHE_FOUND)
    message(STATUS "Found CCACHE")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif()

CPMAddPackage(
        NAME              spdlog
        GITHUB_REPOSITORY gabime/spdlog
        GIT_TAG           v1.15.1)

CPMAddPackage(
        NAME              volk
        GITHUB_REPOSITORY zeux/volk
        GIT_TAG           vulkan-sdk-1.3.296.0)

CPMAddPackage(
        NAME              vk-bootstrap
        GITHUB_REPOSITORY charles-lunarg/vk-bootstrap
        GIT_TAG           v1.3.296
        OPTIONS "VK_BOOTSTRAP_POSITION_INDEPENDENT_CODE ON")

CPMAddPackage(
        NAME              SDL
        GITHUB_REPOSITORY libsdl-org/SDL
        GIT_TAG           release-3.2.2
        OVERRIDE_FIND_PACKAGE)

add_subdirectory(Engine)
add_subdirectory(Runtime)