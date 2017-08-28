find_package(Doxygen REQUIRED)

# Set the doxygen input and output directories
set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR})
set(DOXYGEN_OUTPUT_DIR "${PROJECT_BINARY_DIR}/docs")

# Configure the source Doxyfile by copying it and replacing all @variables@
set(DOXYGEN_CONFIGURED_INPUT "${DOXYGEN_OUTPUT_DIR}/doxyfile")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doxyfile.in"
               ${DOXYGEN_CONFIGURED_INPUT} @ONLY)

# Create the target
add_custom_target(Documentation ALL
                  COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIGURED_INPUT}
                  WORKING_DIRECTORY ${DOXYGEN_INPUT_DIR})

set_target_properties(Documentation PROPERTIES FOLDER "Documentation")
