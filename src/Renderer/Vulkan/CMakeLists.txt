###############################################################################
# Third party libraries

# Vulkan
find_package(Vulkan REQUIRED)

###############################################################################
# Vulkan Renderer

set(ENGINE_VULKAN_PLUGIN Vulkan_Plugin)

engine_add_sources(TARGET PLUGIN_SOURCES
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/Renderer/Vulkan"
                   EXTENSIONS "cpp;hpp")

engine_add_library(TARGET ${ENGINE_VULKAN_PLUGIN}
                   TYPE ${ENGINE_LIBRARY_TYPE}
                   SOURCES "${PLUGIN_SOURCES}")

set(ADDITIONAL_LIBRARIES "")
if(OS STREQUAL "Linux")
    list(APPEND ADDITIONAL_LIBRARIES X11-xcb)
endif()

target_link_libraries(${ENGINE_VULKAN_PLUGIN} ${VULKAN_LIBRARY}
                                              ${ENGINE_LIBRARY}
                                              ${ADDITIONAL_LIBRARIES})
target_include_directories(${ENGINE_VULKAN_PLUGIN} SYSTEM PUBLIC ${VULKAN_INCLUDE_DIR})

set(ENGINE_VULKAN_PLUGIN ${ENGINE_VULKAN_PLUGIN} PARENT_SCOPE)
