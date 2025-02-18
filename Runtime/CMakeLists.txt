project(Runtime)

file(GLOB_RECURSE SOURCES
        LIST_DIRECTORIES true
        ./*.h
        ./*.hpp
        ./*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PUBLIC Engine)
