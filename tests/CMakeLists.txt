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
if(OS STREQUAL "Android")
    add_library(PerlinMap    SHARED "${TEST_DIR}/PerlinMap.cpp")
    add_library(LightCasters SHARED "${TEST_DIR}/LightCasters.cpp")
    add_library(CreateWindow SHARED "${TEST_DIR}/CreateWindow.cpp")
    add_library(LoadModel    SHARED "${TEST_DIR}/LoadModel.cpp")
else()
    add_executable(PerlinMap "${TEST_DIR}/PerlinMap.cpp")
    add_executable(LightCasters "${TEST_DIR}/LightCasters.cpp")
    add_executable(CreateWindow "${TEST_DIR}/CreateWindow.cpp")
    add_executable(LoadModel "${TEST_DIR}/LoadModel.cpp")
endif()
# add_executable(Vulkan "${TEST_DIR}/Vulkan.cpp")

set(TESTS
    # ColorCube
    PerlinMap
    # Camera
    # Light
    # Materials
    # LightingMaps
    LightCasters
    CreateWindow
    LoadModel
    # Vulkan
)

foreach(TEST ${TESTS})
    target_link_libraries(${TEST}
        ${ENGINE_LIBRARY}
        ${ENGINE_OPENGL_PLUGIN}  # TMP
        ${SDL2MAIN_LIBRARY}
    )
endforeach()

target_link_libraries(LoadModel ${ENGINE_LIBRARY} ${SDL2MAIN_LIBRARY})

set_property(TARGET ${TESTS} PROPERTY FOLDER "Tests")

add_subdirectory("${TEST_DIR}/SharedLibrary")
add_subdirectory("${TEST_DIR}/Unitary")
