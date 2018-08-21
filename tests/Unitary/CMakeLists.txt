set(THIS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(TESTS_SOURCES
    "${THIS_DIR}/FileSystemTests.cpp"
    "${THIS_DIR}/SignalTests.cpp"
    "${THIS_DIR}/StringTests.cpp"
    "${THIS_DIR}/TestMain.cpp"
)

add_executable(UnitaryTests ${TESTS_SOURCES})
target_link_libraries(UnitaryTests ${ENGINE_LIBRARY})

set_property(TARGET UnitaryTests PROPERTY FOLDER "Tests")

add_test(NAME UnitaryTests COMMAND UnitaryTests)
