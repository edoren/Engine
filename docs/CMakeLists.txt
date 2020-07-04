find_package(Doxygen REQUIRED)

# Set the doxygen input and output directories
set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR})
set(DOXYGEN_OUTPUT_DIR "${PROJECT_BINARY_DIR}/docs")

# Configure the source Doxyfile by copying it and replacing all @variables@
set(DOXYGEN_CONFIGURED_INPUT "${DOXYGEN_OUTPUT_DIR}/doxyfile")
set(DOXYGEN_MCSS_CONFIGURED_INPUT "${DOXYGEN_OUTPUT_DIR}/doxyfile-mcss")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doxyfile.in" ${DOXYGEN_CONFIGURED_INPUT} @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doxyfile-mcss.in" ${DOXYGEN_MCSS_CONFIGURED_INPUT} @ONLY)

# Run the generator
execute_process(COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIGURED_INPUT}
                WORKING_DIRECTORY ${DOXYGEN_INPUT_DIR})

# Create a custom target to rebuild the docs
add_custom_target(Documentation
                  COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIGURED_INPUT}
                  WORKING_DIRECTORY ${DOXYGEN_INPUT_DIR})


set_target_properties(Documentation PROPERTIES FOLDER "Documentation")
