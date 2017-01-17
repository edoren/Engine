###############################################################################
## CMake configuration

cmake_minimum_required(VERSION 3.1)

# Engine directory
if (NOT DEFINED ENGINE_DIR)
	set (ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

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
if(${OS} STREQUAL "Windows")
    add_definitions(-DUNICODE -D_UNICODE)
endif()

# If compiler is GNU GCC or Clang enable the warnings
if(${COMPILER} STREQUAL "GCC" OR ${COMPILER} STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

###############################################################################
## Third party libraries configuration

set(DEFAULT_OPTION on)
if(${OS} STREQUAL "Windows")
    set(DEFAULT_OPTION off)
endif()

option(USE_SHARED_SDL2 "Use your installed copy of SDL2" ${DEFAULT_OPTION})
option(USE_SHARED_GLEW "Use your installed copy of GLEW" ${DEFAULT_OPTION})

###############################################################################
## Engine configuration

set(ENGINE_NAME Engine)

###############################################################################
## Directories configuration

# Output directories
string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${BUILD_TYPE}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${BUILD_TYPE}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${BUILD_TYPE}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/relwithdebinfo/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/relwithdebinfo/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/relwithdebinfo/bin")

set(THIRD_PARTY_DIR "${ENGINE_DIR}/third_party")
set(ENGINE_SOURCE_DIR "${ENGINE_DIR}/src")
set(TESTS_DIR "${ENGINE_DIR}/tests")
set(TEMP_DIR "${CMAKE_BINARY_DIR}/obj")

###############################################################################

# Set the dependencies folder
add_subdirectory(${THIRD_PARTY_DIR})
include_directories(${THIRD_PARTY_INCLUDES})

###############################################################################

include_directories(${ENGINE_SOURCE_DIR})

# Add the engine folder
add_subdirectory(${ENGINE_SOURCE_DIR})

# Add the test folder
add_subdirectory(${TESTS_DIR})
