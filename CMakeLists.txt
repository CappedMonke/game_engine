cmake_minimum_required(VERSION 3.31)

project(dawns_ballad LANGUAGES CXX)

# Set variables
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES source/main.cpp)

# Add exectutables
add_executable(dawns_ballad ${SOURCES})

# Find packages
find_package(Vulkan REQUIRED)

# Include paths
target_include_directories(dawns_ballad PRIVATE ${Vulkan_INCLUDE_DIRS})

# Link libraries
target_link_libraries(dawns_ballad PRIVATE ${Vulkan_LIBRARIES})