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
    set(SDL2_LIBRARIES ${SDL2_LIBRARY})
else()
    set(SDL_SHARED_ENABLED_BY_DEFAULT OFF)
    add_subdirectory(${SDL2_DIR} "${TEMP_DIR}/sdl2" EXCLUDE_FROM_ALL)
    set(SDL2_INCLUDE_DIR "${SDL2_BINARY_DIR}/include"
                         "${SDL2_SOURCE_DIR}/include")
    set(SDL2_LIBRARIES SDL2main SDL2-static)
    set(COMPILED_DEPENDENCIES ${COMPILED_DEPENDENCIES} ${SDL2_LIBRARIES})
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
    set(GLEW_LIBRARIES glew_s)
    set(COMPILED_DEPENDENCIES ${COMPILED_DEPENDENCIES} ${GLEW_LIBRARIES})
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

# Set the dependencies folder
set_property(TARGET ${COMPILED_DEPENDENCIES} PROPERTY FOLDER "Dependencies")

# Add the dependencies to the parent scope
set(OPENGL_LIBRARIES ${OPENGL_LIBRARIES} PARENT_SCOPE)
set(VULKAN_LIBRARIES ${VULKAN_LIBRARIES} PARENT_SCOPE)
set(SDL2_LIBRARIES ${SDL2_LIBRARIES} PARENT_SCOPE)
set(GLEW_LIBRARIES ${GLEW_LIBRARIES} PARENT_SCOPE)

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
