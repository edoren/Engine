###############################################################################
# Find Vulkan
# https://www.khronos.org/vulkan/
#
# Once done this will define:
#
#   VULKAN_FOUND - System has Vulkan
#   VULKAN_INCLUDE_DIR - The Vulkan include directory
#   VULKAN_LIBRARY - The Vulkan library
#
###############################################################################

if(WIN32)
    set(VULKAN_SHARED_LIBRARY_NAMES vulkan-1)
    set(VULKAN_STATIC_LIBRARY_NAMES vkstatic.1 VKstatic.1)
    set(VULKAN_INCLUDE_PATHS "$ENV{VULKAN_SDK}/Include"
                             "$ENV{VK_SDK_PATH}/Include")
    if(CMAKE_CL_64)
        set(VULKAN_LIBRARY_PATHS "$ENV{VULKAN_SDK}/Bin"
                                 "$ENV{VK_SDK_PATH}/Bin"
                                 "$ENV{VULKAN_SDK}/Lib"
                                 "$ENV{VK_SDK_PATH}/Lib")
    else()
        set(VULKAN_LIBRARY_PATHS "$ENV{VULKAN_SDK}/Bin32"
                                 "$ENV{VK_SDK_PATH}/Bin32"
                                 "$ENV{VULKAN_SDK}/Lib32"
                                 "$ENV{VK_SDK_PATH}/Lib32")
    endif()
else()
    set(VULKAN_SHARED_LIBRARY_NAMES vulkan)
    set(VULKAN_STATIC_LIBRARY_NAMES vkstatic.1 VKstatic.1)
    set(VULKAN_INCLUDE_PATHS "$ENV{VULKAN_SDK}/include"
                             "/usr/include")
    set(VULKAN_LIBRARY_PATHS "$ENV{VULKAN_SDK}/lib"
                             "/usr/lib")
endif()

find_path(VULKAN_INCLUDE_DIR
    NAMES vulkan/vulkan.h
    PATHS ${VULKAN_INCLUDE_PATHS}
)

find_library(VULKAN_SHARED_LIBRARY
    NAMES ${VULKAN_SHARED_LIBRARY_NAMES}
    PATHS ${VULKAN_LIBRARY_PATHS}
)

find_library(VULKAN_STATIC_LIBRARY
    NAMES ${VULKAN_STATIC_LIBRARY_NAMES}
    PATHS ${VULKAN_LIBRARY_PATHS}
)

if(VULKAN_SHARED_LIBRARY)
    set(VULKAN_LIBRARY ${VULKAN_SHARED_LIBRARY})
elseif(VULKAN_STATIC_LIBRARY)
    set(VULKAN_LIBRARY ${VULKAN_STATIC_LIBRARY})
endif()

set(VULKAN_LIBRARIES ${VULKAN_LIBRARY})

if(VULKAN_INCLUDE_DIR)
    set(VULKAN_HEADER_INPUT "${VULKAN_INCLUDE_DIR}/vulkan/vulkan.h")
    file(READ "${VULKAN_HEADER_INPUT}" VULKAN_HEADER_CONTENTS)
    string(REGEX REPLACE ".*#define[ \t]+VK_HEADER_VERSION[ \t]+([0-9]+).*"
           "\\1" VULKAN_HEADER_VERSION "${VULKAN_HEADER_CONTENTS}")
    set(VULKAN_VERSION "1.0.${VULKAN_HEADER_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vulkan
    REQUIRED_VARS
        VULKAN_LIBRARY
        VULKAN_INCLUDE_DIR
    VERSION_VAR
        VULKAN_VERSION
)

mark_as_advanced(
    VULKAN_SHARED_LIBRARY_NAMES
    VULKAN_STATIC_LIBRARY_NAMES
    VULKAN_INCLUDE_PATHS
    VULKAN_LIBRARY_PATHS
    VULKAN_LIBRARIES
    VULKAN_SHARED_LIBRARY
    VULKAN_STATIC_LIBRARY
    VULKAN_HEADER_INPUT
    VULKAN_HEADER_CONTENTS
)
