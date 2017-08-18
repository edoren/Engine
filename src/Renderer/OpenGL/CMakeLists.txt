###############################################################################
# Third party libraries

# Configurable locations of dependencies of this project.
set(GLEW_DIRECTORY "${THIRD_PARTY_DIR}/glew")

###############################################################################
# OpenGL / OpenGL ES Renderer

set(DEPENDENCY_SOURCES "")
set(DEPENDENCY_LIBRARIES ${ENGINE_LIBRARY})
set(DEPENDENCY_INCLUDE_DIRS ${ENGINE_SOURCE_DIR})

if(OS STREQUAL "Android")
    find_package(OpenGLES3)
    list(APPEND DEPENDENCY_LIBRARIES ${OPENGLES3_LIBRARIES})
    list(APPEND DEPENDENCY_INCLUDE_DIRS ${OPENGLES3_INCLUDE_DIR})
else()
    find_package(OpenGL)
    list(APPEND DEPENDENCY_LIBRARIES ${OPENGL_LIBRARY})

    find_package(GLEW)
    if(NOT GLEW_FOUND)
        set(GLEW_INCLUDE_DIR "${GLEW_DIRECTORY}/include")
        set(GLEW_SOURCES "${GLEW_DIRECTORY}/src/glew.c")
        list(APPEND DEPENDENCY_SOURCES ${GLEW_SOURCES})
    else()
        list(APPEND DEPENDENCY_LIBRARIES ${GLEW_LIBRARY})
    endif()
    list(APPEND DEPENDENCY_INCLUDE_DIRS ${GLEW_INCLUDE_DIR})
endif()

set(ENGINE_OPENGL_PLUGIN OpenGL_Plugin)

engine_add_sources(TARGET PLUGIN_SOURCES
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}/Renderer/OpenGL"
                   EXTENSIONS "cpp;hpp")

engine_add_library(TARGET ${ENGINE_OPENGL_PLUGIN}
                   TYPE ${ENGINE_LIBRARY_TYPE}
                   SOURCES ${PLUGIN_SOURCES} ${DEPENDENCY_SOURCES})

# TMP: GLEW should not export any symbols when compiled internally
if(ENGINE_BUILD_STATIC)
    target_compile_definitions(${ENGINE_OPENGL_PLUGIN} PRIVATE GLEW_STATIC=1)
else()
    target_compile_definitions(${ENGINE_OPENGL_PLUGIN} PRIVATE GLEW_BUILD=1)
endif()

target_link_libraries(${ENGINE_OPENGL_PLUGIN} ${DEPENDENCY_LIBRARIES})
target_include_directories(${ENGINE_OPENGL_PLUGIN} SYSTEM PUBLIC ${DEPENDENCY_INCLUDE_DIRS})

set(ENGINE_OPENGL_PLUGIN ${ENGINE_OPENGL_PLUGIN} PARENT_SCOPE)
