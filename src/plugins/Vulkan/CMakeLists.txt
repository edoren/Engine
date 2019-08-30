###############################################################################
# Vulkan Renderer

set(VULKAN_PLUGIN_BUILD_STATIC FALSE)

find_package(Vulkan)

if(VULKAN_PLUGIN_BUILD_STATIC)
    set(BUILD_TYPE_NAME "STATIC")
else()
    set(BUILD_TYPE_NAME "SHARED")
endif()

if(NOT VULKAN_FOUND)
    message(FATAL_ERROR "Vulkan not found. Make sure to install the Vulkan SDK "
                        "and set the VULKAN_SDK envoriment variable to point "
                        "to it.")
endif()

engine_add_sources(TARGET PLUGIN_SOURCES
                   BASE_FOLDER "${ENGINE_PLUGINS_DIR}/Vulkan")

engine_add_library(TARGET Vulkan_Plugin
                   TYPE ${BUILD_TYPE_NAME}
                   SOURCES ${PLUGIN_SOURCES})

target_link_libraries(Vulkan_Plugin
    ${VULKAN_LIBRARY}
    ${ENGINE_LIBRARY}
)

target_include_directories(Vulkan_Plugin SYSTEM
    PRIVATE ${VULKAN_INCLUDE_DIR}
    PRIVATE ${ENGINE_INCLUDE_DIR}
)

set(ENGINE_VULKAN_PLUGIN Vulkan_Plugin PARENT_SCOPE)
