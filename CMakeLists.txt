cmake_minimum_required(VERSION 3.31)

project(dawns_ballad LANGUAGES CXX)

# Set variables
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add exectutable
file(GLOB_RECURSE SOURCES source/*.cpp)
add_executable(dawns_ballad ${SOURCES})

# Include headers
target_include_directories(dawns_ballad PRIVATE ${CMAKE_SOURCE_DIR}/source)

# Include Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(dawns_ballad PRIVATE ${Vulkan_LIBRARIES})
target_include_directories(dawns_ballad PRIVATE ${Vulkan_INCLUDE_DIRS})

# Enable FetchContent module and set the base directory
include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/external)

# Fetch and include SDL3 (if not already present)
set(SDL3_SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/sdl3-src)
set(SDL3_BINARY_DIR ${FETCHCONTENT_BASE_DIR}/sdl3-build)
if (NOT EXISTS ${SDL3_SOURCE_DIR})
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG preview-3.1.6
    )
    FetchContent_MakeAvailable(SDL3)
else()
    message(STATUS "SDL3 already fetched")
    add_subdirectory(${SDL3_SOURCE_DIR} ${SDL3_BINARY_DIR})
endif()
target_link_libraries(dawns_ballad PRIVATE SDL3::SDL3)
target_include_directories(dawns_ballad PRIVATE ${SDL3_SOURCE_DIR}/include)