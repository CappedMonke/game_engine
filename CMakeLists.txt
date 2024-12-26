cmake_minimum_required(VERSION 3.31)

project(engine LANGUAGES CXX)

# Set variables
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(engine engine/source/main.cpp)

# Find packages
find_package(Vulkan REQUIRED)

# Include paths
target_include_directories(engine PRIVATE ${Vulkan_INCLUDE_DIRS})

# Link libraries
target_link_libraries(engine PRIVATE ${Vulkan_LIBRARIES})