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

# OpenGL Renderer
set(ENGINE_RENDER_OPENGL_LIBRARY OpenGL_Plugin)

engine_add_sources(TARGET ENGINE_RENDER_OPENGL_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/Renderer/OpenGL"
                   EXTENSIONS "cpp;hpp")

engine_add_library(TARGET ${ENGINE_RENDER_OPENGL_LIBRARY} ${ENGINE_LIBRARY_TYPE}
                   SOURCES "${ENGINE_RENDER_OPENGL_SRCS};${GLEW_SOURCES}"
                   DEPENDENCIES ${OPENGL_LIBRARY} ${ENGINE_LIBRARY})

target_include_directories(${ENGINE_RENDER_OPENGL_LIBRARY} SYSTEM PUBLIC ${GLEW_INCLUDE_DIR})
target_compile_definitions(${ENGINE_RENDER_OPENGL_LIBRARY} PRIVATE GLEW_STATIC=1)

if(GLEW_FOUND)
    target_link_libraries(${ENGINE_RENDER_OPENGL_LIBRARY} ${GLEW_LIBRARY})
endif()

set(ENGINE_RENDER_OPENGL_LIBRARY ${ENGINE_RENDER_OPENGL_LIBRARY} PARENT_SCOPE)
