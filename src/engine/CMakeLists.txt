###############################################################################
## Engine library

engine_add_sources(TARGET ENGINE_SRCS
                   BASE_FOLDER "${ENGINE_SOURCE_DIR}")

engine_create_filters("${ENGINE_SRCS}")

set(ENGINE_LIBRARY Engine)

engine_add_library(TARGET ${ENGINE_LIBRARY}
                   TYPE ${ENGINE_LIBRARY_TYPE}
                   SOURCES "${ENGINE_SRCS}")

target_include_directories(${ENGINE_LIBRARY} PUBLIC ${ENGINE_INCLUDE_DIR})
target_include_directories(${ENGINE_LIBRARY} SYSTEM PUBLIC ${THIRD_PARTY_INCLUDES})

set_target_properties(${ENGINE_LIBRARY} PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(OS_WINDOWS)
    target_link_libraries(${ENGINE_LIBRARY}
        ${SDL2MAIN_LIBRARY}
        ${SDL2_LIBRARY}
        ${ASSIMP_LIBRARY}
        imm32
        version
    )
elseif(OS_LINUX)
    target_link_libraries(${ENGINE_LIBRARY}
        dl
        pthread
        sndio
        z
    )
elseif(OS_ANDROID)
    target_link_libraries(${ENGINE_LIBRARY}
        ${SDL2_LIBRARY}
        ${ASSIMP_LIBRARY}
        android
        log
    )
elseif(OS_IOS)
    target_link_libraries(${ENGINE_LIBRARY}
        ${SDL2_LIBRARY}
        ${ASSIMP_LIBRARY}
    )
elseif(OS_MACOS)
    target_link_libraries(${ENGINE_LIBRARY}
        ${SDL2_LIBRARY}
        ${ASSIMP_LIBRARY}
    )
endif()

set(ENGINE_LIBRARY ${ENGINE_LIBRARY} PARENT_SCOPE)
