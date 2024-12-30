cmake_minimum_required(VERSION 3.31)

project(dawns_ballad LANGUAGES CXX)

# Set variables
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SOURCES 
    source/main.cpp 
    source/graphics/render_manager.cpp
)

# Add exectutables
add_executable(dawns_ballad ${SOURCES})

# Include Vulkan
find_package(Vulkan REQUIRED)
target_include_directories(dawns_ballad PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(dawns_ballad PRIVATE ${Vulkan_LIBRARIES})

# Include GLFW
include(FetchContent)
FetchContent_Declare(
    GLFW
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(GLFW)
target_link_libraries(dawns_ballad PRIVATE GLFW)