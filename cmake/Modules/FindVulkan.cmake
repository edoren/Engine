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
                                 "$ENV{VK_SDK_PATH}/Bin")
    else()
        set(VULKAN_LIBRARY_PATHS "$ENV{VULKAN_SDK}/Bin32"
                                 "$ENV{VK_SDK_PATH}/Bin32")
    endif()
else()
    set(VULKAN_SHARED_LIBRARY_NAMES vulkan)
    set(VULKAN_STATIC_LIBRARY_NAMES vkstatic.1 VKstatic.1)
    set(VULKAN_INCLUDE_PATHS "$ENV{VULKAN_SDK}/include")
    set(VULKAN_LIBRARY_PATHS "$ENV{VULKAN_SDK}/lib")
endif()

find_path(VULKAN_INCLUDE_DIR
    NAMES vulkan/vulkan.h
    HINTS ${VULKAN_INCLUDE_PATHS}
)

find_library(VULKAN_SHARED_LIBRARY
    NAMES ${VULKAN_SHARED_LIBRARY_NAMES}
    HINTS ${VULKAN_LIBRARY_PATHS}
)

find_library(VULKAN_STATIC_LIBRARY
    NAMES ${VULKAN_STATIC_LIBRARY_NAMES}
    HINTS ${VULKAN_LIBRARY_PATHS}
)

if(VULKAN_SHARED_LIBRARY)
    set(VULKAN_LIBRARY ${VULKAN_SHARED_LIBRARY})
elseif(VULKAN_STATIC_LIBRARY)
    set(VULKAN_LIBRARY ${VULKAN_STATIC_LIBRARY})
endif()

set(VULKAN_LIBRARIES ${VULKAN_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vulkan DEFAULT_MSG VULKAN_LIBRARY
                                                     VULKAN_INCLUDE_DIR)

mark_as_advanced(
    VULKAN_SHARED_LIBRARY_NAMES
    VULKAN_STATIC_LIBRARY_NAMES
    VULKAN_INCLUDE_PATHS
    VULKAN_LIBRARY_PATHS
    VULKAN_INCLUDE_DIR
    VULKAN_LIBRARY
    VULKAN_LIBRARIES
    VULKAN_SHARED_LIBRARY
    VULKAN_STATIC_LIBRARY
)

