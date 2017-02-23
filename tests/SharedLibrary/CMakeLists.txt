set(THIS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Create a test executable
add_library(OperationAdd SHARED "${THIS_DIR}/OperationAdd.cpp")
add_library(OperationMult SHARED "${THIS_DIR}/OperationMult.cpp")
add_executable(SharedLibTest "${THIS_DIR}/SharedLibTest.cpp")

target_link_libraries(SharedLibTest ${ENGINE_LIBRARY})

set_property(TARGET SharedLibTest OperationAdd OperationMult
             PROPERTY FOLDER "Tests/SharedLibrary")

add_test(NAME SharedLibTest COMMAND SharedLibTest)
