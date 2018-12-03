set(TEST_DIR "${CMAKE_SOURCE_DIR}/tests")

# Configure unit tests via CTest
enable_testing()

# Create a test executable
# add_executable(ColorCube "${TEST_DIR}/ColorCube.cpp")
# add_executable(PerlinMap "${TEST_DIR}/PerlinMap.cpp")
# add_executable(Camera "${TEST_DIR}/Camera.cpp")
# add_executable(Light "${TEST_DIR}/Light.cpp")
# add_executable(Materials "${TEST_DIR}/Materials.cpp")
# add_executable(LightingMaps "${TEST_DIR}/LightingMaps.cpp")
if(OS_ANDROID)
    # add_library(PerlinMap    SHARED "${TEST_DIR}/PerlinMap.cpp")
    # add_library(LightCasters SHARED "${TEST_DIR}/LightCasters.cpp")
    add_library(CreateWindow SHARED "${TEST_DIR}/CreateWindow.cpp")
    add_library(LoadModel    SHARED "${TEST_DIR}/LoadModel.cpp")
else()
    # add_executable(PerlinMap "${TEST_DIR}/PerlinMap.cpp")
    # add_executable(LightCasters "${TEST_DIR}/LightCasters.cpp")
    add_executable(CreateWindow "${TEST_DIR}/CreateWindow.cpp")
    add_executable(LoadModel "${TEST_DIR}/LoadModel.cpp")
endif()
# add_executable(Vulkan "${TEST_DIR}/Vulkan.cpp")

set(TESTS
    # ColorCube
    # PerlinMap
    # Camera
    # Light
    # Materials
    # LightingMaps
    # LightCasters
    CreateWindow
    LoadModel
    # Vulkan
)

foreach(TEST ${TESTS})
    if(OS_WINDOWS)
        target_link_libraries(${TEST}
            ${SDL2MAIN_LIBRARY}
            ${ENGINE_LIBRARY}
            ${SDL2_LIBRARY}
            ${ASSIMP_LIBRARY}
        )
    elseif(OS_LINUX)
        target_link_libraries(${TEST}
            ${SDL2MAIN_LIBRARY}
            "-Wl,--whole-archive"
            ${ENGINE_LIBRARY}
            "-Wl,--no-whole-archive"
            ${SDL2_LIBRARY}
            ${ASSIMP_LIBRARY}
        )
    elseif(OS_ANDROID)
        target_link_libraries(${TEST}
            # ${SDL2MAIN_LIBRARY}
            ${ENGINE_LIBRARY}
        )
    elseif(OS_MACOS)
        target_link_libraries(${TEST}
            # ${SDL2MAIN_LIBRARY}
            ${ENGINE_LIBRARY}
        )
    endif()
endforeach()

set_property(TARGET ${TESTS} PROPERTY FOLDER "Tests")

# add_subdirectory("${TEST_DIR}/SharedLibrary")
if(OS_WINDOWS OR OS_LINUX OR OS_MACOS)
    add_subdirectory("${TEST_DIR}/Unitary")
endif()
