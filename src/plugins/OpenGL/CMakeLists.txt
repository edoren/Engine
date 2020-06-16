###############################################################################
# OpenGL / OpenGL ES Renderer

set(OPENGL_PLUGIN_BUILD_STATIC FALSE)

if(OS_ANDROID OR OS_IOS)
    find_package(OpenGLES3)
else()
    find_package(OpenGL)
endif()

if(NOT OPENGL_FOUND AND NOT OPENGLES3_FOUND)
    message(FATAL_ERROR "OpenGL or OpenGLES3 not found")
endif()

if(OS_ANDROID)
    set(DEPENDENCY_LIBRARIES
        ${OPENGLES3_LIBRARIES}
    )
    set(DEPENDENCY_INCLUDE_DIRS
        ${OPENGLES3_INCLUDE_DIR}
    )
else()
    set(DEPENDENCY_LIBRARIES
        ${OPENGL_LIBRARIES}
    )
    set(DEPENDENCY_INCLUDE_DIRS
        ${OPENGL_INCLUDE_DIR}
    )
endif()

# Windows, Linux and Mac implementation use GLEW
if(OS_WINDOWS OR OS_LINUX OR OS_MACOS)
    set(GLEW_SOURCES "${THIRD_PARTY_DIR}/src/GLEW/glew.c")
    set(DEPENDENCY_SOURCES ${GLEW_SOURCES})
endif()

if(OPENGL_PLUGIN_BUILD_STATIC)
    set(BUILD_TYPE_NAME "STATIC")
else()
    set(BUILD_TYPE_NAME "SHARED")
endif()

if (DEPENDENCY_SOURCES)
    engine_add_library(TARGET Dependencies_Object
                       TYPE "OBJECT"
                       SOURCES ${${DEPENDENCY_SOURCES}})
    # set_target_properties(Dependencies_Object
    #                       PROPERTIES CXX_CLANG_TIDY "")
    target_include_directories(Dependencies_Object SYSTEM
        PRIVATE ${DEPENDENCY_INCLUDE_DIRS}
        PRIVATE ${ENGINE_INCLUDE_DIR}
    )
endif()

engine_add_sources(TARGET PLUGIN_SOURCES
                   BASE_FOLDER "${ENGINE_PLUGINS_DIR}/OpenGL")

engine_add_library(TARGET OpenGL_Plugin
                   TYPE ${BUILD_TYPE_NAME}
                   SOURCES ${PLUGIN_SOURCES})

if (Dependencies_Object)
    target_sources(OpenGL_Plugin $<TARGET_OBJECTS:Dependencies_Object>)
endif()

# GLEW should not export any symbols when compiled internally
if(OPENGL_PLUGIN_BUILD_STATIC)
    target_compile_definitions(OpenGL_Plugin PRIVATE GLEW_STATIC=1)
else()
    target_compile_definitions(OpenGL_Plugin PRIVATE GLEW_BUILD=1)
endif()

target_link_libraries(OpenGL_Plugin
    ${DEPENDENCY_LIBRARIES}
    ${ENGINE_LIBRARY}
)

target_include_directories(OpenGL_Plugin SYSTEM
    PRIVATE ${DEPENDENCY_INCLUDE_DIRS}
    PRIVATE ${ENGINE_INCLUDE_DIR}
)

set(ENGINE_OPENGL_PLUGIN OpenGL_Plugin PARENT_SCOPE)
