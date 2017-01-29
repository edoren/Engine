set(THIS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Create a test executable
add_library(Impl1 SHARED "${THIS_DIR}/Impl1.cpp")
add_library(Impl2 SHARED "${THIS_DIR}/Impl2.cpp")
add_executable(SharedLibTest "${THIS_DIR}/Main.cpp")

target_link_libraries(SharedLibTest
    ${ENGINE_LIBRARY}
)

set_property(TARGET SharedLibTest Impl1 Impl2 PROPERTY FOLDER "Tests/SharedLibrary")
