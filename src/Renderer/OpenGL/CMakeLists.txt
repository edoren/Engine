###############################################################################
# Third party libraries

# Configurable locations of dependencies of this project.
set(GLEW_DIRECTORY "${THIRD_PARTY_DIR}/glew")

# OpenGL
find_package(OpenGL REQUIRED)

# GLEW
find_package(GLEW)
if(NOT GLEW_FOUND)
    set(GLEW_INCLUDE_DIR "${GLEW_DIRECTORY}/include")
    set(GLEW_SOURCES "${GLEW_DIRECTORY}/src/glew.c")
endif()

###############################################################################
# OpenGL Renderer

set(ENGINE_OPENGL_PLUGIN OpenGL_Plugin)

engine_add_sources(TARGET PLUGIN_SOURCES
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/Renderer/OpenGL"
                   EXTENSIONS "cpp;hpp")

if(NOT GLEW_FOUND)
    list(APPEND PLUGIN_SOURCES ${GLEW_SOURCES})
endif()

engine_add_library(TARGET ${ENGINE_OPENGL_PLUGIN}
                   TYPE ${ENGINE_LIBRARY_TYPE}
                   SOURCES "${PLUGIN_SOURCES}")

target_link_libraries(${ENGINE_OPENGL_PLUGIN} ${OPENGL_LIBRARY} ${ENGINE_LIBRARY})
target_include_directories(${ENGINE_OPENGL_PLUGIN} SYSTEM PUBLIC ${GLEW_INCLUDE_DIR})

# TMP: GLEW should not export any symbols when compiled internally
if(ENGINE_BUILD_STATIC)
    target_compile_definitions(${ENGINE_OPENGL_PLUGIN} PRIVATE GLEW_STATIC=1)
else()
    target_compile_definitions(${ENGINE_OPENGL_PLUGIN} PRIVATE GLEW_BUILD=1)
endif()

if(GLEW_FOUND)
    target_link_libraries(${ENGINE_OPENGL_PLUGIN} ${GLEW_LIBRARY})
endif()

set(ENGINE_OPENGL_PLUGIN ${ENGINE_OPENGL_PLUGIN} PARENT_SCOPE)
