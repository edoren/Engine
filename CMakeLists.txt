###############################################################################
## CMake configuration

cmake_minimum_required(VERSION 2.8.12)

# Engine directory
if (NOT DEFINED ENGINE_DIR)
    set (ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# Set the default Android NDK STL to LLVM libc++
set(ANDROID_STL "c++_static")

project(Engine)

# Configure the projects to use folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Limit the build configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo"
    CACHE STRING "Limited configurations" FORCE)
mark_as_advanced(CMAKE_CONFIGURATION_TYPES)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug
        CACHE STRING "Choose the type of build, options are: Debug Release"
        FORCE)
endif()

# Set the custom module path
list(APPEND CMAKE_MODULE_PATH "${ENGINE_DIR}/cmake/Modules")

# Include the config and macros file
include("${ENGINE_DIR}/cmake/Config.cmake")
include("${ENGINE_DIR}/cmake/Macros.cmake")

###############################################################################
## Compiler configuration

# Require C++11 support
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Unicode in Windows
if(OS STREQUAL "Windows")
    add_definitions(-DUNICODE -D_UNICODE)
endif()

# If compiler is GNU GCC or Clang enable the warnings
if(COMPILER STREQUAL "GCC" OR COMPILER STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

###############################################################################
## Engine configuration

set(ENGINE_NAME Engine)

option(ENGINE_BUILD_STATIC "Build the Engine as an static library" OFF)

if(ENGINE_BUILD_STATIC)
    add_definitions(-DENGINE_STATIC)
    set(ENGINE_LIBRARY_TYPE STATIC)
else()
    set(ENGINE_LIBRARY_TYPE SHARED)
endif()

###############################################################################
## Directories configuration

# Output directories
if(NOT ANDROID)
    string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${BUILD_TYPE}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${BUILD_TYPE}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${BUILD_TYPE}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/relwithdebinfo")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/relwithdebinfo")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/relwithdebinfo")
endif()

set(THIRD_PARTY_DIR "${ENGINE_DIR}/third_party")
set(ENGINE_SOURCE_DIR "${ENGINE_DIR}/src")
set(TESTS_DIR "${ENGINE_DIR}/tests")
set(TEMP_DIR "${CMAKE_BINARY_DIR}/obj")

###############################################################################

# Set the dependencies folder
add_subdirectory(${THIRD_PARTY_DIR})
include_directories(SYSTEM ${THIRD_PARTY_INCLUDES})

###############################################################################

include_directories(${ENGINE_SOURCE_DIR})

# Add the engine folder
add_subdirectory(${ENGINE_SOURCE_DIR})

# Build the renderers
add_subdirectory("${ENGINE_SOURCE_DIR}/Renderer/OpenGL")
add_subdirectory("${ENGINE_SOURCE_DIR}/Renderer/Vulkan")

# Add the test folder
add_subdirectory(${TESTS_DIR})

