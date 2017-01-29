set(TEST_DIR "${CMAKE_SOURCE_DIR}/tests")

# Create a test executable
# add_executable(ColorCube "${TEST_DIR}/ColorCube.cpp")
# add_executable(PerlinMap "${TEST_DIR}/PerlinMap.cpp")
# add_executable(Camera "${TEST_DIR}/Camera.cpp")
# add_executable(Light "${TEST_DIR}/Light.cpp")
# add_executable(Materials "${TEST_DIR}/Materials.cpp")
# add_executable(LightingMaps "${TEST_DIR}/LightingMaps.cpp")
add_executable(LightCasters "${TEST_DIR}/LightCasters.cpp")
# add_executable(Vulkan "${TEST_DIR}/Vulkan.cpp")

set(TESTS
    # ColorCube
    # PerlinMap
    # Camera
    # Light
    # Materials
    # LightingMaps
    LightCasters
    # Vulkan
)

foreach(TEST ${TESTS})
    target_link_libraries(${TEST}
        # ${OPENGL_LIBRARY}
        # ${VULKAN_LIBRARY}
        # ${SDL2_LIBRARY}
        # ${GLEW_LIBRARY}
        ${ENGINE_LIBRARY}
    )
endforeach()

add_custom_command(
    TARGET ${TESTS} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/data" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data"
)

set_property(TARGET ${TESTS} PROPERTY FOLDER "Tests")

add_subdirectory("${TEST_DIR}/SharedLibrary")
