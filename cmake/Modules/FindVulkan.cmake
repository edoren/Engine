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
    # Search for the default VulkanSDK installation folder
    set(VK_DEFAULT_INSTALL_PATH "C:/VulkanSDK")
    if(EXISTS ${VK_DEFAULT_INSTALL_PATH})
        file(GLOB VK_SDK_SUBDIR_CONTENTS "${VK_DEFAULT_INSTALL_PATH}/*")
        foreach(VK_SDK_SUBDIR ${VK_SDK_SUBDIR_CONTENTS})
            if(IS_DIRECTORY ${VK_SDK_SUBDIR})
                list(APPEND VK_INCLUDE_PATHS "${VK_SDK_SUBDIR}/Include")
                list(APPEND VK_LIB64_PATHS "${VK_SDK_SUBDIR}/Bin" "${VK_SDK_SUBDIR}/Lib")
                list(APPEND VK_LIB_PATHS "${VK_SDK_SUBDIR}/Bin32" "${VK_SDK_SUBDIR}/Lib32")
            endif()
        endforeach()
    endif()

    # Use the VULKAN_SDK or VK_SDK_PATH environment variable
    set(VULKAN_SHARED_LIBRARY_NAMES vulkan-1)
    set(VULKAN_STATIC_LIBRARY_NAMES vkstatic.1 VKstatic.1)
    set(VULKAN_INCLUDE_PATHS ${VK_INCLUDE_PATHS}
                             "$ENV{VULKAN_SDK}/Include"
                             "$ENV{VK_SDK_PATH}/Include")
    if(CMAKE_CL_64)
        set(VULKAN_LIBRARY_PATHS ${VK_LIB64_PATHS}
                                 "$ENV{VULKAN_SDK}/Bin"
                                 "$ENV{VK_SDK_PATH}/Bin"
                                 "$ENV{VULKAN_SDK}/Lib"
                                 "$ENV{VK_SDK_PATH}/Lib")
    else()
        set(VULKAN_LIBRARY_PATHS ${VK_LIB_PATHS}
                                 "$ENV{VULKAN_SDK}/Bin32"
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
    set(VULKAN_HEADER_INPUT "${VULKAN_INCLUDE_DIR}/vulkan/vulkan_core.h")
    if(NOT EXISTS ${VULKAN_HEADER_INPUT})
        set(VULKAN_HEADER_INPUT "${VULKAN_INCLUDE_DIR}/vulkan/vulkan.h")
    endif()

    set(VULKAN_API_VERSION "1.0")
    set(VULKAN_HEADER_VERSION "0")
    file(STRINGS "${VULKAN_HEADER_INPUT}" VULKAN_HEADER_CONTENTS)
    foreach(SRC_LINE ${VULKAN_HEADER_CONTENTS})
        if("${SRC_LINE}" MATCHES "^#define[ \t]+VK_API_VERSION_([0-9]+)_([0-9]+).*$")
            set(FOUND_VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}")
            if(${VULKAN_API_VERSION} LESS ${FOUND_VERSION})
                set(VULKAN_API_VERSION "${FOUND_VERSION}")
            endif()
        endif()
        if("${SRC_LINE}" MATCHES "^#define[ \t]+VK_HEADER_VERSION[ \t]+([0-9]+).*$")
            set(VULKAN_HEADER_VERSION "${CMAKE_MATCH_1}")
        endif()
    endforeach()

    set(VULKAN_VERSION "${VULKAN_API_VERSION}.${VULKAN_HEADER_VERSION}")
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
