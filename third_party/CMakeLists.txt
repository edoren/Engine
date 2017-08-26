###############################################################################
# Global third party libraries

# Configurable locations of dependencies of this project.
set(SDL2_DIR "${THIRD_PARTY_DIR}/sdl2")
set(ASSIMP_DIR "${THIRD_PARTY_DIR}/assimp")

# SDL2
find_package(SDL2 2.0.6)
if (NOT SDL2_FOUND)
    # set(SDL_SHARED_ENABLED_BY_DEFAULT OFF)
    set(SDL2_BINARY_DIR "${TEMP_DIR}/sdl2")
    add_subdirectory(${SDL2_DIR} "${SDL2_BINARY_DIR}" EXCLUDE_FROM_ALL)
    set(SDL2_INCLUDE_DIR "${SDL2_BINARY_DIR}/include"
                         "${SDL2_SOURCE_DIR}/include")
    # Remove this file to force the compiler to use the generated SDL_config.h
    file(REMOVE "${SDL2_SOURCE_DIR}/include/SDL_config.h")
    set(SDL2MAIN_LIBRARY SDL2main)
    if(ENGINE_BUILD_STATIC)
        set(SDL2_LIBRARY SDL2-static)
    else()
        set(SDL2_LIBRARY SDL2)
    endif()
    set_property(TARGET ${SDL2MAIN_LIBRARY} ${SDL2_LIBRARY}
                 PROPERTY FOLDER "ThirdParty")
endif()

# Assimp
find_package(Assimp)
if (NOT ASSIMP_FOUND)
    set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT FALSE CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_OBJ_IMPORTER TRUE CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_PLY_IMPORTER TRUE CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_FBX_IMPORTER TRUE CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_BLEND_IMPORTER TRUE CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_COLLADA_IMPORTER TRUE CACHE BOOL "" FORCE)
    # Always use assimp as an static library
    set(BUILD_SHARED_LIBS_TMP ${BUILD_SHARED_LIBS})
    set(BUILD_SHARED_LIBS FALSE CACHE BOOL "" FORCE)
    set(ASSIMP_BINARY_DIR "${TEMP_DIR}/assimp")
    set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_TMP})
    add_subdirectory(${ASSIMP_DIR} "${ASSIMP_BINARY_DIR}" EXCLUDE_FROM_ALL)
    set(ASSIMP_LIBRARY assimp)
    set(ASSIMP_INCLUDE_DIR "${ASSIMP_DIR}/include"
                           "${ASSIMP_BINARY_DIR}/include")
endif()

###############################################################################

# Add the dependencies to the parent scope
set(SDL2MAIN_LIBRARY ${SDL2MAIN_LIBRARY} PARENT_SCOPE)
set(SDL2_LIBRARY ${SDL2_LIBRARY} PARENT_SCOPE)
set(ASSIMP_LIBRARY ${ASSIMP_LIBRARY} PARENT_SCOPE)

# Set the third party include directories

set(THIRD_PARTY_INCLUDES
    ${SDL2_INCLUDE_DIR}
    ${ASSIMP_INCLUDE_DIR}
    PARENT_SCOPE)
