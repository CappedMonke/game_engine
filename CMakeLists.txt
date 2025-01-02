cmake_minimum_required(VERSION 3.31)

project(dawns_ballad LANGUAGES CXX)

# Set variables
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add exectutable
set(SOURCES 
    source/main.cpp 
    source/graphics/render_manager.cpp
)
add_executable(dawns_ballad ${SOURCES})

# Include Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(dawns_ballad PRIVATE ${Vulkan_LIBRARIES})
target_include_directories(dawns_ballad PRIVATE ${Vulkan_INCLUDE_DIRS})

# Enable FetchContent module and set the base directory
include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/external)

# Fetch and include SDL3
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG preview-3.1.6
)
FetchContent_MakeAvailable(SDL3)
target_link_libraries(dawns_ballad PRIVATE SDL3::SDL3)
target_include_directories(dawns_ballad PRIVATE ${SDL3_SOURCE_DIR}/include)