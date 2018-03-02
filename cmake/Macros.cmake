include(CMakeParseArguments)

include("${CMAKE_CURRENT_LIST_DIR}/Config.cmake")

# Macro to create the filters for Visual Studio and other IDEs
macro(engine_create_filters SOURCES)
    foreach(FILE ${SOURCES})
        get_filename_component(FILE_EXTENSION "${FILE}" EXT)
        get_filename_component(FILE_DIRECTORY "${FILE}" DIRECTORY)

        file(RELATIVE_PATH FILE_RELATIVE_DIR ${ENGINE_SOURCE_DIR} ${FILE_DIRECTORY})

        string(REPLACE "/" "\\" FILTER ${FILE_RELATIVE_DIR})
        if(${FILE_EXTENSION} MATCHES "\.(c|cpp|m|mm)$")
            set(FILTER "Source Files\\${FILTER}")
        elseif(${FILE_EXTENSION} MATCHES "\.(h|hpp)$")
            set(FILTER "Header Files\\${FILTER}")
        endif()
        source_group(${FILTER} FILES ${FILE})
    endforeach()
endmacro(engine_create_filters)

function(_engine_find_files TARGET DIRECTORY EXTENSIONS)
    foreach(EXTENSION ${EXTENSIONS})
        file(GLOB TMP_FILES "${DIRECTORY}/*.${EXTENSION}")
        list(APPEND FILES ${TMP_FILES})
    endforeach()
    set(${TARGET} "${FILES}" PARENT_SCOPE)
endfunction(_engine_find_files)

function(engine_add_sources)
    set(options IGNORE_BASE_FOLDER IGNORE_SYSTEM_FOLDERS)
    set(one_val_args TARGET BASE_FOLDER)
    set(multi_val_args MODULES EXTENSIONS)

    cmake_parse_arguments(THIS "${options}" "${one_val_args}" "${multi_val_args}" ${ARGN})

    if(NOT THIS_TARGET)
        message(FATAL_ERROR "TARGET argument not specified.")
    endif()

    if(NOT THIS_BASE_FOLDER)
        message(FATAL_ERROR "BASE_FOLDER argument must be declared.")
    endif()

    if(NOT EXTENSIONS)
        set(EXTENSIONS c cpp h hpp)
        if(OS_MACOS OR OS_IOS)
            list(APPEND EXTENSIONS m mm)
        endif()
    endif()

    if(NOT IGNORE_BASE_FOLDER)
        _engine_find_files(TMP_SOURCES "${THIS_BASE_FOLDER}" "${EXTENSIONS}")
        list(APPEND SOURCES "${TMP_SOURCES}")
    endif()

    foreach(MODULE ${THIS_MODULES})
        set(FOLDER "${THIS_BASE_FOLDER}/${MODULE}")
        _engine_find_files(TMP_SOURCES "${FOLDER}" "${EXTENSIONS}")
        list(APPEND SOURCES "${TMP_SOURCES}" "${TMP_OS_SOURCES}")
    endforeach()

    set(${THIS_TARGET} "${SOURCES}" PARENT_SCOPE)
endfunction(engine_add_sources)

macro(engine_add_library)
    set(options "")
    set(one_val_args TARGET TYPE NAME FOLDER)
    set(multi_val_args SOURCES DEPENDENCIES)

    cmake_parse_arguments(THIS "${options}" "${one_val_args}" "${multi_val_args}" ${ARGN})

    if(NOT THIS_TARGET)
        message(FATAL_ERROR "TARGET argument not specified.")
    endif()

    if(NOT THIS_NAME)
        string(REPLACE "_" "-" NAME_LOWER "${THIS_TARGET}")
        string(TOLOWER "${NAME_LOWER}" THIS_NAME)
    endif()

    if (NOT THIS_TYPE)
        message(FATAL_ERROR "TYPE argument not specified.")
    endif()
    string(TOUPPER "${THIS_TYPE}" THIS_TYPE)

    # Create the library
    if(THIS_TYPE STREQUAL "SHARED")
        add_library(${THIS_TARGET} SHARED ${THIS_SOURCES})
        set_target_properties(${THIS_TARGET} PROPERTIES DEBUG_POSTFIX "-d")
        if (OS_WINDOWS AND COMPILER_GCC)
            # On Windows using GCC get rid of "lib" prefix for shared libraries,
            # and transform the ".dll.a" suffix into ".a" for import libraries
            set_target_properties(${THIS_TARGET} PROPERTIES PREFIX "")
            set_target_properties(${THIS_TARGET} PROPERTIES IMPORT_SUFFIX ".a")
        endif()
    elseif(THIS_TYPE STREQUAL "STATIC")
        add_library(${THIS_TARGET} STATIC ${THIS_SOURCES})
        set_target_properties(${THIS_TARGET} PROPERTIES DEBUG_POSTFIX "-s-d")
        set_target_properties(${THIS_TARGET} PROPERTIES RELEASE_POSTFIX "_s")
    elseif(THIS_TYPE STREQUAL "OBJECT")
        add_library(${THIS_TARGET} OBJECT ${THIS_SOURCES})
    else()
        message(FATAL_ERROR "TYPE argument invalid values, it must be SHARED, STATIC or OBJECT")
    endif()

    # Set the library name
    set_target_properties(${THIS_TARGET} PROPERTIES OUTPUT_NAME ${THIS_NAME})

    # Create the exports symbol
    string(REPLACE "-" "_" NAME_UPPER "${THIS_NAME}")
    string(TOUPPER "${NAME_UPPER}" NAME_UPPER)
    set_target_properties(${THIS_TARGET} PROPERTIES DEFINE_SYMBOL ${NAME_UPPER}_EXPORTS)

    # Add the library to a folder
    if(THIS_FOLDER)
        set_target_properties(${THIS_TARGET} PROPERTIES FOLDER ${THIS_FOLDER})
    endif()
endmacro()
