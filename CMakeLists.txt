###############################################################################
## CMake configuration

cmake_minimum_required(VERSION 3.3)

# Engine directory
if(NOT DEFINED ENGINE_DIR)
    set (ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

if(NOT APP_NAME)
    message(FATAL_ERROR "APP_NAME not set")
endif()

project(${APP_NAME})

# Configure the projects to use folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Limit the build configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
mark_as_advanced(CMAKE_CONFIGURATION_TYPES)

# Select the build type for the application
if(APP_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE ${APP_BUILD_TYPE} CACHE STRING "" FORCE)
else()
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

# Set the custom module path
set(CMAKE_MODULE_PATH "${ENGINE_DIR}/cmake/Modules")

# Limit the build configurations
# set(CMAKE_CXX_CLANG_TIDY "clang-tidy" ${CLANG_TIDY_COMMANDS})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include the config and macros file
include("Config")
include("Macros")

###############################################################################
## Compiler configuration

# Require C++14 support
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Unicode in Windows
if(OS_WINDOWS)
    add_definitions(-DUNICODE -D_UNICODE)
endif()

# If compiler is GNU GCC or Clang enable the warnings
if(COMPILER_GCC OR COMPILER_CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()
if(COMPILER_MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /wd4201")
endif()

###############################################################################
## Engine configuration

set(ENGINE_NAME Engine)

option(ENGINE_BUILD_STATIC "Build the Engine as an static library" OFF)
option(ENGINE_BUILD_TESTS "Build the Engine test projects" ON)
option(ENGINE_BUILD_DOCS "Build the Engine documentation (Requires Doxygen)" OFF)

if(ENGINE_BUILD_STATIC)
    add_definitions(-DENGINE_STATIC)
    set(ENGINE_LIBRARY_TYPE STATIC)
else()
    set(ENGINE_LIBRARY_TYPE SHARED)
endif()

###############################################################################
## Directories configuration

# Output directories
if(NOT OS_ANDROID)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release")
endif()

set(THIRD_PARTY_DIR "${ENGINE_DIR}/third_party")
set(ENGINE_SOURCE_DIR "${ENGINE_DIR}/src/engine")
set(ENGINE_INCLUDE_DIR "${ENGINE_DIR}/src/engine")
set(ENGINE_PLUGINS_DIR "${ENGINE_DIR}/src/plugins")
set(TESTS_DIR "${ENGINE_DIR}/tests")
set(DOCS_DIR "${ENGINE_DIR}/docs")

###############################################################################

# Set the dependencies folder
if(OS_WINDOWS)
    set(CMAKE_C_FLAGS_DEBUG "/MD /Zi /Ob0 /Od /RTC1")
    set(CMAKE_CXX_FLAGS_DEBUG "/MD /Zi /Ob0 /Od /RTC1")
    set(SDL2MAIN_LIBRARY ${THIRD_PARTY_DIR}/prebuilt/windows/Release/SDL2main.lib)
    set(SDL2_LIBRARY ${THIRD_PARTY_DIR}/prebuilt/windows/Release/SDL2.lib)
    set(ASSIMP_LIBRARY ${THIRD_PARTY_DIR}/prebuilt/windows/Release/assimp.lib
                       ${THIRD_PARTY_DIR}/prebuilt/windows/Release/zlibstatic.lib
                       ${THIRD_PARTY_DIR}/prebuilt/windows/Release/IrrXML.lib)
elseif(OS_LINUX)
    set(SDL2MAIN_LIBRARY ${THIRD_PARTY_DIR}/prebuilt/linux/Release/libSDL2main.a)
    set(SDL2_LIBRARY ${THIRD_PARTY_DIR}/prebuilt/linux/Release/libSDL2.a)
    set(ASSIMP_LIBRARY ${THIRD_PARTY_DIR}/prebuilt/linux/Release/libassimp.a)
elseif(OS_ANDROID)
    set(SDL2MAIN_LIBRARY ${THIRD_PARTY_DIR}/prebuilt/android/${ANDROID_ABI}/Release/libSDL2main.a)
    set(SDL2_LIBRARY ${THIRD_PARTY_DIR}/prebuilt/android/${ANDROID_ABI}/Release/libSDL2.a
                     ${THIRD_PARTY_DIR}/prebuilt/android/${ANDROID_ABI}/Release/libhidapi.so)
    set(ASSIMP_LIBRARY ${THIRD_PARTY_DIR}/prebuilt/android/${ANDROID_ABI}/Release/libassimp.a z)
elseif(OS_IOS)
    set(SDL2MAIN_LIBRARY ${THIRD_PARTY_DIR}/prebuilt/ios/Release/libSDL2main.a)
    set(SDL2_LIBRARY ${THIRD_PARTY_DIR}/prebuilt/ios/Release/libSDL2.a
                    "-framework CoreBluetooth"
                    "-framework CoreGraphics"
                    "-framework CoreMotion"
                    "-framework CoreVideo"
                    "-framework Foundation"
                    "-framework GameController"
                    "-framework OpenGLES"
                    "-framework QuartzCore"
                    "-framework UIKit"
                    iconv)
    set(ASSIMP_LIBRARY ${THIRD_PARTY_DIR}/prebuilt/ios/Release/libassimp.a
                       z
                       ${THIRD_PARTY_DIR}/prebuilt/ios/Release/libIrrXML.a)
elseif(OS_MACOS)
    set(SDL2MAIN_LIBRARY ${THIRD_PARTY_DIR}/prebuilt/macos/Release/libSDL2main.a)
    set(SDL2_LIBRARY ${THIRD_PARTY_DIR}/prebuilt/macos/Release/libSDL2.a
                    "-framework Cocoa"
                    "-framework ForceFeedback"
                    "-framework IOKit"
                    "-framework CoreVideo"
                    "-framework Carbon"
                    iconv)
    set(ASSIMP_LIBRARY ${THIRD_PARTY_DIR}/prebuilt/macos/Release/libassimp.a
                       z
                       ${THIRD_PARTY_DIR}/prebuilt/macos/Release/libIrrXML.a)
endif()

if(OS_WINDOWS)
    list(APPEND THIRD_PARTY_INCLUDES "${THIRD_PARTY_DIR}/include/windows")
elseif(OS_LINUX)
    list(APPEND THIRD_PARTY_INCLUDES "${THIRD_PARTY_DIR}/include/linux")
elseif(OS_ANDROID)
    list(APPEND THIRD_PARTY_INCLUDES "${THIRD_PARTY_DIR}/include/android/${ANDROID_ABI}")
elseif(OS_IOS)
    list(APPEND THIRD_PARTY_INCLUDES "${THIRD_PARTY_DIR}/include/ios")
elseif(OS_MACOS)
    list(APPEND THIRD_PARTY_INCLUDES "${THIRD_PARTY_DIR}/include/macos")
endif()
list(APPEND THIRD_PARTY_INCLUDES "${THIRD_PARTY_DIR}/include/common")

###############################################################################
## Engine library

add_subdirectory("${ENGINE_SOURCE_DIR}")

###############################################################################
## Build the plugins

set(ENGINE_PLUGINS OpenGL Vulkan)
foreach(PLUGIN ${ENGINE_PLUGINS})
    add_subdirectory("${ENGINE_PLUGINS_DIR}/${PLUGIN}")
endforeach()

###############################################################################
## Tests

if(ENGINE_BUILD_TESTS)
    add_subdirectory(${TESTS_DIR})
endif()

set(CMAKE_XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${APP_PACKAGE}")
set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "$ENV{DEVELOPMENT_TEAM_ID}")
set_target_properties(
    ${APP_NAME}
    PROPERTIES
        XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path @executable_path/Frameworks @loader_path/Frameworks"
)

###############################################################################
## Documentation

if(ENGINE_BUILD_DOCS)
    add_subdirectory(${DOCS_DIR})
endif()

###############################################################################
## Data processing

if(NOT PYTHON_EXECUTABLE)
    find_package(PythonInterp 3 REQUIRED)
endif()

set(SCRIPTS_FOLDER "${CMAKE_SOURCE_DIR}/scripts")

set(DATA_SOURCE_FOLDER "${CMAKE_SOURCE_DIR}/data")
if(OS_ANDROID)
    set(DATA_OUTPUT_FOLDER "${CMAKE_SOURCE_DIR}/projects/android/app/src/main/assets")
elseif(OS_IOS)
    set(DATA_OUTPUT_FOLDER "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${APP_NAME}.app/data")
else()
    set(DATA_OUTPUT_FOLDER "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data")
endif()

file(GLOB_RECURSE DATA_FILES "${DATA_SOURCE_FOLDER}/*.*")

add_custom_command(
    OUTPUT ${DATA_OUTPUT_FOLDER}
    COMMAND ${PYTHON_EXECUTABLE} "${SCRIPTS_FOLDER}/process_data.py" ${DATA_SOURCE_FOLDER} ${DATA_OUTPUT_FOLDER}
    DEPENDS ${DATA_FILES}
)

add_custom_target(
    BuildAssets ALL
    DEPENDS ${DATA_OUTPUT_FOLDER}
)
