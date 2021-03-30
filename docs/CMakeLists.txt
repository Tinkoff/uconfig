cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

## Make sure that custom modules are found
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED breathe exhale)

set(PROJECT_AUTHOR "i.s.vovk@tinkoff.ru")
set(PROJECT_VERSION ${CPPCONFIG_VERSION_STRING})

set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR}/include)
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
set(DOXYGEN_STRIP_PATH ${DOXYGEN_INPUT_DIR})

set(EXHALE_REL_OUTPUT_DIR api)
set(EXHALE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${EXHALE_REL_OUTPUT_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in ${CMAKE_CURRENT_BINARY_DIR}/conf.py @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/index.rst.in ${CMAKE_CURRENT_BINARY_DIR}/index.rst @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/genindex.rst.in ${CMAKE_CURRENT_BINARY_DIR}/genindex.rst @ONLY)

set(HTML_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/html)
add_custom_target(docs
                  COMMAND ${SPHINX_BUILD_EXECUTABLE} -b html -j auto . ${HTML_OUTPUT_DIR}
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                  COMMENT "Generating documentation with sphinx"
)
