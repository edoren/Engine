###############################################################################
## Third party libraries

# Configurable locations of dependencies of this project.
set(MATHFU_DIR "${THIRD_PARTY_DIR}/mathfu")
set(STB_DIR "${THIRD_PARTY_DIR}/stb")
set(UTFCPP_DIR "${THIRD_PARTY_DIR}/utfcpp")
set(FMT_DIR "${THIRD_PARTY_DIR}/fmt")

# Mathfu
set(MATHFU_INCLUDE_DIR "${MATHFU_DIR}/include"
                       "${MATHFU_DIR}/dependencies/vectorial/include")

# STB
set(STB_INCLUDE_DIR ${STB_DIR})

# UTFCPP
set(UTFCPP_INCLUDE_DIR "${UTFCPP_DIR}/source")

# FMT
set(FMT_INCLUDE_DIR "${FMT_DIR}")

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
                   MODULES FileSystem IO
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

target_link_libraries(${ENGINE_LIBRARY} ${SDL2_LIBRARY})

target_include_directories(${ENGINE_LIBRARY} SYSTEM
                           PUBLIC ${MATHFU_INCLUDE_DIR} ${FMT_INCLUDE_DIR} # TMP?
                           PRIVATE ${STB_INCLUDE_DIR} ${UTFCPP_INCLUDE_DIR})

set(ENGINE_LIBRARY ${ENGINE_LIBRARY} PARENT_SCOPE)
