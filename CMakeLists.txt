cmake_minimum_required(VERSION 3.6...3.31)

project(dawns_ballad LANGUAGES CXX)

# ==================================================================================================
# Set C++ standard
# ==================================================================================================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==================================================================================================
# Add executable
# ==================================================================================================
file(GLOB_RECURSE sources source/*.cpp)
add_executable(${PROJECT_NAME} ${sources})

# ==================================================================================================
# Include project source directory
# ==================================================================================================
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/source)

# ==================================================================================================
# Include Vulkan
# ==================================================================================================
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})

# ==================================================================================================
# Enable FetchContent module
# ==================================================================================================
include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/external)

# ==================================================================================================
# Fetch and include SDL3
# ==================================================================================================
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG preview-3.1.8
)
FetchContent_MakeAvailable(SDL3)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL3_SOURCE_DIR}/include)

# ==================================================================================================
# Fetch and include GLM
# ==================================================================================================
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)