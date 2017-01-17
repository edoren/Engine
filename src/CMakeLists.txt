set(EXTENSIONS c cpp h hpp)

## Math sources
engine_add_sources(TARGET ENGINE_MATH_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/Math"
                   MODULES Noise
                   EXTENSIONS "${EXTENSIONS}")

## System sources
engine_add_sources(TARGET ENGINE_SYSTEM_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/System"
                   MODULES DLL FileSystem IO
                   EXTENSIONS "${EXTENSIONS}")

## Utils sources
engine_add_sources(TARGET ENGINE_UTIL_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/Util"
                   EXTENSIONS "${EXTENSIONS}")

## Render sources
engine_add_sources(TARGET ENGINE_RENDER_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/Renderer"
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

set(ENGINE_SRCS
    ${ENGINE_MATH_SRCS}
    ${ENGINE_SYSTEM_SRCS}
    ${ENGINE_UTIL_SRCS}
    ${ENGINE_RENDER_SRCS}
    ${ENGINE_GRAPHICS_SRCS}
    ${ENGINE_INPUT_SRCS}
)

engine_create_filters("${ENGINE_SRCS}")

set(ENGINE_LIBRARY engine)
engine_add_library(TARGET ${ENGINE_LIBRARY} STATIC
                   SOURCES "${ENGINE_SRCS}"
                   DEPENDENCIES ${OPENGL_LIBRARIES}
                                ${SDL2_LIBRARIES}
                                ${GLEW_LIBRARIES})
set_target_properties(${ENGINE_LIBRARY} PROPERTIES DEFINE_SYMBOL "ENGINE_EXPORTS")

set(ENGINE_LIBRARIES ${ENGINE_LIBRARY} PARENT_SCOPE)
