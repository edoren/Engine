# Configurable locations of dependencies of this project.
set(SDL2_DIR "${THIRD_PARTY_DIR}/sdl2")
set(GLEW_DIRECTORY "${THIRD_PARTY_DIR}/glew")
set(MATHFU_DIR "${THIRD_PARTY_DIR}/mathfu")
set(STB_DIR "${THIRD_PARTY_DIR}/stb")
set(UTFCPP_DIR "${THIRD_PARTY_DIR}/utfcpp")


###############################################################################

# OpenGL
find_package(OpenGL REQUIRED)

# Vulkan
find_package(Vulkan REQUIRED)

# SDL2
if (USE_SHARED_SDL2)
    find_package(SDL2 REQUIRED)
    set(SDL2_LIBRARY ${SDL2_LIBRARY})
else()
    # set(SDL_SHARED_ENABLED_BY_DEFAULT OFF)
    add_subdirectory(${SDL2_DIR} "${TEMP_DIR}/sdl2" EXCLUDE_FROM_ALL)
    set(SDL2_INCLUDE_DIR "${SDL2_BINARY_DIR}/include"
                         "${SDL2_SOURCE_DIR}/include")
    set(SDL2_MAIN_LIBRARY SDL2main)
    set(SDL2_SHARED_LIBRARY SDL2)
    set(SDL2_STATIC_LIBRARY SDL2-static)
    list(APPEND COMPILED_THIRD_PARTY ${SDL2_SHARED_LIBRARY}
                                     ${SDL2_MAIN_LIBRARY}
                                     ${SDL2_STATIC_LIBRARY})
    # (hack) We redefine install to stop SDL's CMakeLists.txt from
    # referring to SDL2 lib
    function(install)
    endfunction(install)
endif()

# GLEW
if (USE_SHARED_GLEW)
    find_package(GLEW)
else()
    add_subdirectory(
        "${GLEW_DIRECTORY}/build/cmake"
        "${TEMP_DIR}/glew"
        EXCLUDE_FROM_ALL
    )
    set(GLEW_INCLUDE_DIR "${GLEW_DIRECTORY}/include")
    set(GLEW_LIBRARY glew_s)
    list(APPEND COMPILED_THIRD_PARTY ${GLEW_LIBRARY})
    add_definitions(-DGLEW_STATIC)
endif()

# Mathfu
set(MATHFU_INCLUDE_DIR "${MATHFU_DIR}/include"
                       "${MATHFU_DIR}/dependencies/vectorial/include")

# STB
set(STB_INCLUDE_DIR ${STB_DIR})

# UTFCPP
set(UTFCPP_INCLUDE_DIR "${UTFCPP_DIR}/source")

###############################################################################

# Set the thirdparty folder
set_property(TARGET ${COMPILED_THIRD_PARTY} PROPERTY FOLDER "ThirdParty")

# Add the dependencies to the parent scope
set(OPENGL_LIBRARY ${OPENGL_LIBRARY} PARENT_SCOPE)
set(VULKAN_LIBRARY ${VULKAN_LIBRARY} PARENT_SCOPE)
set(SDL2_MAIN_LIBRARY ${SDL2_MAIN_LIBRARY} PARENT_SCOPE)
set(SDL2_SHARED_LIBRARY ${SDL2_SHARED_LIBRARY} PARENT_SCOPE)
set(SDL2_STATIC_LIBRARY ${SDL2_STATIC_LIBRARY} PARENT_SCOPE)
set(GLEW_LIBRARY ${GLEW_LIBRARY} PARENT_SCOPE)

# Set the third party include directories
set(THIRD_PARTY_INCLUDES
        ${OPENGL_INCLUDE_DIR}
        ${SDL2_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIR}
        ${MATHFU_INCLUDE_DIR}
        ${STB_INCLUDE_DIR}
        ${UTFCPP_INCLUDE_DIR}
        ${VULKAN_INCLUDE_DIR}
    PARENT_SCOPE)
