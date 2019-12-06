###############################################################################
# Find OpenGL ES 3 and EGL
# https://www.khronos.org/vulkan/
#
# If using ARM Mali emulation you can specify the parent directory that contains the bin and include directories by
# setting the MALI_SDK_ROOT variable in the environment.
#
# For AMD emulation use the AMD_SDK_ROOT variable
#
# Once done this will define
#
#  OPENGLES3_FOUND        - system has OpenGLES
#  OPENGLES3_INCLUDE_DIR  - the GL include directory
#  OPENGLES3_LIBRARIES    - Link these to use OpenGLES
#
#  EGL_FOUND        - system has EGL
#  EGL_INCLUDE_DIR  - the EGL include directory
#  EGL_LIBRARIES    - Link these to use EGL
#
###############################################################################

if(WIN32)
    if(CYGWIN)

        find_path(OPENGLES3_INCLUDE_DIR GLES3/gl3.h)

        find_library(OPENGLES3_GL_LIBRARY libGLESv3)

    else(CYGWIN)

        if(BORLAND)
            set (OPENGLES3_GL_LIBRARY import32 CACHE STRING "OpenGL ES 3.x library for win32")
        else(BORLAND)
            set(POWERVR_SDK_PATH "C:/Imagination/PowerVR/GraphicsSDK/SDK_3.1/Builds")
            find_path(OPENGLES3_INCLUDE_DIR
                NAMES GLES3/gl3.h
                PATHS $ENV{AMD_SDK_ROOT}/include
                      $ENV{MALI_SDK_ROOT}/include
                      ${POWERVR_SDK_PATH}/Include
            )

            find_path(EGL_INCLUDE_DIR
                NAMES EGL/egl.h
                PATHS $ENV{AMD_SDK_ROOT}/include
                      $ENV{MALI_SDK_ROOT}/include
                      ${POWERVR_SDK_PATH}/Include
            )

            find_library(OPENGLES3_GL_LIBRARY
                NAMES libGLESv3
                PATHS $ENV{AMD_SDK_ROOT}/x86
                    $ENV{MALI_SDK_ROOT}/bin
                    ${POWERVR_SDK_PATH}/Windows/x86_32/Lib
            )

            find_library(EGL_egl_LIBRARY
                NAMES libEGL
                PATHS $ENV{AMD_SDK_ROOT}/x86
                    $ENV{MALI_SDK_ROOT}/bin
                    ${POWERVR_SDK_PATH}/Windows/x86_32/Lib
            )
        endif(BORLAND)

  endif(CYGWIN)

else(WIN32)

    if(APPLE)

        # create_search_paths(/Developer/Platforms)
        find_library(OPENGLES3_GL_LIBRARY_FRAMEWORK NAMES OpenGLES3 OpenGLES)
        if(OPENGLES3_GL_LIBRARY_FRAMEWORK MATCHES "\.framework$")
            set(OPENGLES3_INCLUDE_DIR ${OPENGLES3_GL_LIBRARY_FRAMEWORK}/Headers)
            set(OPENGLES3_GL_LIBRARY ${OPENGLES3_GL_LIBRARY_FRAMEWORK})
        endif()

    else(APPLE)
        find_path(OPENGLES3_INCLUDE_DIR
            NAMES GLES3/gl3.h
            PATHS $ENV{AMD_SDK_ROOT}/include
                  $ENV{MALI_SDK_ROOT}/include
                  /opt/Imagination/PowerVR/GraphicsSDK/SDK_3.1/Builds/Include
                  /usr/openwin/share/include
                  /opt/graphics/OpenGL/include /usr/X11R6/include
                  /usr/include
        )

        find_library(OPENGLES3_GL_LIBRARY
            NAMES GLESv3
            PATHS $ENV{AMD_SDK_ROOT}/x86
                  $ENV{MALI_SDK_ROOT}/bin
                  /opt/Imagination/PowerVR/GraphicsSDK/SDK_3.1/Builds/Linux/x86_32/Lib
                  /opt/graphics/OpenGL/lib
                  /usr/openwin/lib
                  /usr/shlib /usr/X11R6/lib
                  /usr/lib
        )

        find_path(EGL_INCLUDE_DIR
            NAMES EGL/egl.h
            PATHS $ENV{AMD_SDK_ROOT}/include
                  $ENV{MALI_SDK_ROOT}/include
                  /opt/Imagination/PowerVR/GraphicsSDK/SDK_3.1/Builds/Include
                  /usr/openwin/share/include
                  /opt/graphics/OpenGL/include /usr/X11R6/include
                  /usr/include
        )

        find_library(EGL_egl_LIBRARY
            NAMES EGL
            PATHS $ENV{AMD_SDK_ROOT}/x86
                  $ENV{MALI_SDK_ROOT}/bin
                  /opt/Imagination/PowerVR/GraphicsSDK/SDK_3.1/Builds/Linux/x86_32/Lib
                  /opt/graphics/OpenGL/lib
                  /usr/openwin/lib
                  /usr/shlib /usr/X11R6/lib
                  /usr/lib
        )

        # On Unix OpenGL most certainly always requires X11.
        # Feel free to tighten up these conditions if you don't
        # think this is always true.
        # It's not true on OSX.

        if(OPENGLES3_GL_LIBRARY)
            if(NOT X11_FOUND)
                INCLUDE(FindX11)
            endif(NOT X11_FOUND)
            if(X11_FOUND)
                if(NOT APPLE)
                set (OPENGLES3_LIBRARIES ${X11_LIBRARIES})
                endif(NOT APPLE)
            endif(X11_FOUND)
        endif(OPENGLES3_GL_LIBRARY)

    endif(APPLE)
endif(WIN32)

set( OPENGLES3_FOUND "YES" )
if(OPENGLES3_GL_LIBRARY)

    set( OPENGLES3_LIBRARIES ${OPENGLES3_GL_LIBRARY} ${OPENGLES3_LIBRARIES})
    if(EGL_egl_LIBRARY)
        set( EGL_LIBRARIES ${EGL_egl_LIBRARY} ${EGL_LIBRARIES})
    endif()
    set( OPENGLES3_FOUND "YES" )

endif(OPENGLES3_GL_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenGLES3 DEFAULT_MSG OPENGLES3_GL_LIBRARY
                                                        OPENGLES3_INCLUDE_DIR)

mark_as_advanced(
    OPENGLES3_INCLUDE_DIR
    OPENGLES3_GL_LIBRARY
    EGL_INCLUDE_DIR
    EGL_egl_LIBRARY
)
