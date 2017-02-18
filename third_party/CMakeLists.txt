###############################################################################
# Global third party libraries

# Configurable locations of dependencies of this project.
set(SDL2_DIR "${THIRD_PARTY_DIR}/sdl2")

# SDL2
find_package(SDL2)
if (NOT SDL2_FOUND)
    set(SDL2_SHARED_LIBRARY ${SDL2_LIBRARY})
else()
    # set(SDL_SHARED_ENABLED_BY_DEFAULT OFF)
    add_subdirectory(${SDL2_DIR} "${TEMP_DIR}/sdl2" EXCLUDE_FROM_ALL)
    set(SDL2_INCLUDE_DIR "${SDL2_BINARY_DIR}/include"
                         "${SDL2_SOURCE_DIR}/include")
    set(SDL2MAIN_LIBRARY SDL2main)
    set(SDL2_SHARED_LIBRARY SDL2)
    set(SDL2_STATIC_LIBRARY SDL2-static)
    set_property(TARGET ${SDL2MAIN_LIBRARY}
                        ${SDL2_SHARED_LIBRARY}
                        ${SDL2_STATIC_LIBRARY}
                 PROPERTY
                     FOLDER "ThirdParty")
endif()

###############################################################################

# Add the dependencies to the parent scope
set(SDL2MAIN_LIBRARY ${SDL2MAIN_LIBRARY} PARENT_SCOPE)
set(SDL2_SHARED_LIBRARY ${SDL2_SHARED_LIBRARY} PARENT_SCOPE)
set(SDL2_STATIC_LIBRARY ${SDL2_STATIC_LIBRARY} PARENT_SCOPE)

# Set the third party include directories

set(THIRD_PARTY_INCLUDES ${SDL2_INCLUDE_DIR} PARENT_SCOPE)
