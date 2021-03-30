cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

set(UCONFIG_VERSION_MAJOR "2")
set(UCONFIG_VERSION_MINOR "0")
set(UCONFIG_VERSION_RELEASE "3")
set(UCONFIG_SUMMARY "C++ library")
set(UCONFIG_DESCRIPTION "C++ library to parse and emit multi-format configuration for your app")
set(UCONFIG_REPOSITORY_URL "https://github.com/TinkoffCreditSystems/uconfig")
set(UCONFIG_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(UCONFIG_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(UCONFIG_VERSION_STRING "${UCONFIG_VERSION_MAJOR}.${UCONFIG_VERSION_MINOR}.${UCONFIG_VERSION_RELEASE}")

## Make sure that custom modules are found
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

project(uconfig VERSION "${UCONFIG_VERSION_STRING}" LANGUAGES CXX)

##############################################
# Declare options and project-wide parameters

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-class-memaccess")

## Compile as RelWithDebInfo info by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

option(UCONFIG_BUILD_TESTING "Build included unit-tests" OFF)
option(BUILD_DOCS "Build sphinx generated docs" OFF)

##############################################
# Create target and set properties

## libuconfig
add_library(uconfig INTERFACE)
add_library(${PROJECT_NAME}::uconfig ALIAS uconfig)

target_include_directories(uconfig
    INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

find_package(RapidJSON)
if (RapidJSON_FOUND)
    target_include_directories(uconfig INTERFACE ${RapidJSON_INCLUDE_DIRS})
    target_compile_definitions(uconfig INTERFACE RAPIDJSON_HAS_STDSTRING=1)
endif()

##############################################
# Installation instructions

include(GNUInstallDirs)

## Install targets
install(TARGETS
    uconfig
    EXPORT uconfigTargets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

## Install headers
install(DIRECTORY
    include/uconfig
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

## Configure cmake helpers
include(CMakePackageConfigHelpers)
set(UCONFIG_VERSION_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/uconfigConfigVersion.cmake")
set(UCONFIG_PROJECT_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/uconfigConfig.cmake")
set(UCONFIG_CMAKE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/uconfig/")
write_basic_package_version_file(${UCONFIG_VERSION_CONFIG}
    VERSION ${UCONFIG_VERSION_STRING}
    COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(${PROJECT_SOURCE_DIR}/uconfigConfig.cmake.in
    ${UCONFIG_PROJECT_CONFIG}
    INSTALL_DESTINATION ${UCONFIG_CMAKE_INSTALL_DIR}
)

## Install cmake helpers
install(FILES
    ${UCONFIG_PROJECT_CONFIG}
    ${UCONFIG_VERSION_CONFIG}
    DESTINATION ${UCONFIG_CMAKE_INSTALL_DIR}
)
install(EXPORT
    uconfigTargets
    DESTINATION ${UCONFIG_CMAKE_INSTALL_DIR}
    NAMESPACE ${PROJECT_NAME}::
)
## Exporting from the build tree
export(EXPORT
    uconfigTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/uconfigTargets.cmake
    NAMESPACE ${PROJECT_NAME}::
)

## Register package in user's package registry
export(PACKAGE ${PROJECT_NAME})

##############################################
# Unit-tests

if(UCONFIG_BUILD_TESTING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")

    include(CTest)
    add_subdirectory(tests)
endif()

##############################################
# Docs

if(BUILD_DOCS)
    add_subdirectory(docs)
endif()
