###############################################################################
## Engine library

set(EXTENSIONS c cpp h hpp)

# Math sources
engine_add_sources(TARGET ENGINE_MATH_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/Math"
                   MODULES Noise
                   EXTENSIONS "${EXTENSIONS}")

# System sources
engine_add_sources(TARGET ENGINE_SYSTEM_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/System"
                   EXTENSIONS "${EXTENSIONS}")

# Utils sources
engine_add_sources(TARGET ENGINE_UTIL_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/Util"
                   EXTENSIONS "${EXTENSIONS}")

# Render sources
engine_add_sources(TARGET ENGINE_RENDER_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/Renderer"
                   EXTENSIONS "${EXTENSIONS}")

# Graphic sources
engine_add_sources(TARGET ENGINE_GRAPHICS_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/Graphics"
                   MODULES 3D
                   EXTENSIONS "${EXTENSIONS}")

# Input sources
engine_add_sources(TARGET ENGINE_INPUT_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/Input"
                   EXTENSIONS "${EXTENSIONS}")

# Core sources
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

engine_add_library(TARGET ${ENGINE_LIBRARY}
                   TYPE ${ENGINE_LIBRARY_TYPE}
                   SOURCES "${ENGINE_SRCS}")

if(OS_WINDOWS)
    target_link_libraries(${ENGINE_LIBRARY}
        ${SDL2_LIBRARY}
        ${ASSIMP_LIBRARY}
        imm32
        version
    )
elseif(OS_LINUX)
    target_link_libraries(${ENGINE_LIBRARY}
        dl
        pthread
        sndio
        z
    )
elseif(OS_ANDROID)
    target_link_libraries(${ENGINE_LIBRARY}
        ${SDL2_LIBRARY}
        ${ASSIMP_LIBRARY}
        android
        log
    )
elseif(OS_IOS)
    target_link_libraries(${ENGINE_LIBRARY}
        ${SDL2_LIBRARY}
        ${ASSIMP_LIBRARY}
    )
elseif(OS_MACOS)
    target_link_libraries(${ENGINE_LIBRARY}
        ${SDL2_LIBRARY}
        ${ASSIMP_LIBRARY}
    )
endif()

set_target_properties(${ENGINE_LIBRARY} PROPERTIES POSITION_INDEPENDENT_CODE ON)

set(ENGINE_LIBRARY ${ENGINE_LIBRARY} PARENT_SCOPE)
