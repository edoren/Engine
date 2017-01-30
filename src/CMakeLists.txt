set(EXTENSIONS c cpp h hpp)

## Math sources
engine_add_sources(TARGET ENGINE_MATH_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/Math"
                   MODULES Noise
                   EXTENSIONS "${EXTENSIONS}")

## System sources
engine_add_sources(TARGET ENGINE_SYSTEM_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/System"
                   MODULES FileSystem IO
                   EXTENSIONS "${EXTENSIONS}")

## Utils sources
engine_add_sources(TARGET ENGINE_UTIL_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/Util"
                   EXTENSIONS "${EXTENSIONS}")

## Render sources
engine_add_sources(TARGET ENGINE_RENDER_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/Renderer"
                   MODULES OpenGL
                   EXTENSIONS "${EXTENSIONS}")

## Graphic sources
engine_add_sources(TARGET ENGINE_GRAPHICS_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/Graphics"
                   MODULES 3D
                   EXTENSIONS "${EXTENSIONS}")

## Input sources
engine_add_sources(TARGET ENGINE_INPUT_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/Input"
                   EXTENSIONS "${EXTENSIONS}")

## Core sources
engine_add_sources(TARGET ENGINE_CORE_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/Core"
                   EXTENSIONS "${EXTENSIONS}")

set(ENGINE_SRCS
    ${ENGINE_MATH_SRCS}
    ${ENGINE_SYSTEM_SRCS}
    ${ENGINE_UTIL_SRCS}
    ${ENGINE_RENDER_SRCS}
    ${ENGINE_GRAPHICS_SRCS}
    ${ENGINE_INPUT_SRCS}
    ${ENGINE_CORE_SRCS}
)

engine_create_filters("${ENGINE_SRCS}")

set(ENGINE_LIBRARY Engine)

if(ENGINE_BUILD_STATIC)
    set(ENGINE_LIBRARY_TYPE STATIC)
else()
    set(ENGINE_LIBRARY_TYPE SHARED)
endif()

engine_add_library(TARGET ${ENGINE_LIBRARY} ${ENGINE_LIBRARY_TYPE}
                   SOURCES "${ENGINE_SRCS}"
                   DEPENDENCIES ${OPENGL_LIBRARY}
                                ${VULKAN_LIBRARY}
                                ${SDL2_MAIN_LIBRARY}
                                "${SDL2_${ENGINE_LIBRARY_TYPE}_LIBRARY}"
                                ${GLEW_LIBRARY})

set(ENGINE_LIBRARY ${ENGINE_LIBRARY} PARENT_SCOPE)
