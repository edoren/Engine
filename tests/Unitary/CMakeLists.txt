set(THIS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(TESTS_SOURCES
    "${THIS_DIR}/FileSystemTests.cpp"
    "${THIS_DIR}/SignalTests.cpp"
    "${THIS_DIR}/StringTests.cpp"
    "${THIS_DIR}/TestMain.cpp"
)

add_executable(UnitaryTests ${TESTS_SOURCES})
if(OS_WINDOWS)
    target_link_libraries(UnitaryTests
        ${SDL2MAIN_LIBRARY}
        ${ENGINE_LIBRARY}
        ${SDL2_LIBRARY}
        ${ASSIMP_LIBRARY}
    )
elseif(OS_LINUX)
    target_link_libraries(UnitaryTests
        ${SDL2MAIN_LIBRARY}
        "-Wl,--whole-archive"
        ${ENGINE_LIBRARY}
        "-Wl,--no-whole-archive"
        ${SDL2_LIBRARY}
        ${ASSIMP_LIBRARY}
    )
elseif(OS_ANDROID)
    target_link_libraries(UnitaryTests
        # ${SDL2MAIN_LIBRARY}
        ${ENGINE_LIBRARY}
    )
elseif(OS_MACOS)
    target_link_libraries(UnitaryTests
        # ${SDL2MAIN_LIBRARY}
        ${ENGINE_LIBRARY}
    )
endif()

set_property(TARGET UnitaryTests PROPERTY FOLDER "Tests")

add_test(NAME UnitaryTests COMMAND UnitaryTests)
