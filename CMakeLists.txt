cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

set(UCONFIG_VERSION_MAJOR "1")
set(UCONFIG_VERSION_MINOR "0")
set(UCONFIG_VERSION_RELEASE "1")
set(UCONFIG_VERSION_STRING "${UCONFIG_VERSION_MAJOR}.${UCONFIG_VERSION_MINOR}.${UCONFIG_VERSION_RELEASE}")
set(UCONFIG_LIB_VERSION ${UCONFIG_VERSION_STRING})
mark_as_advanced(UCONFIG_VERSION_MAJOR UCONFIG_VERSION_MINOR UCONFIG_VERSION_RELEASE UCONFIG_VERSION_STRING UCONFIG_LIB_VERSION)

project(uconfig VERSION "${UCONFIG_VERSION_STRING}" LANGUAGES CXX)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# compile in release with debug info mode by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# If uconfig is being bundled in another project, we don't want to
# install anything.
if(NOT DEFINED UCONFIG_BUNDLED_MODE)
  if("${PROJECT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    set(UCONFIG_BUNDLED_MODE OFF)
  else()
    set(UCONFIG_BUNDLED_MODE ON)
  endif()
endif()
set(UCONFIG_BUNDLED_MODE ${UCONFIG_BUNDLED_MODE} CACHE BOOL "" FORCE)
mark_as_advanced(UCONFIG_BUNDLED_MODE)

# libuconfig
add_library(uconfig INTERFACE)
add_library(${PROJECT_NAME}::uconfig ALIAS uconfig)

find_package(Boost 1.65 REQUIRED)

target_include_directories(uconfig
    INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    INTERFACE ${Boost_INCLUDE_DIRS}
)

if(NOT UCONFIG_BUNDLED_MODE)
    include(GNUInstallDirs)

    install(TARGETS uconfig
        EXPORT uconfigTargets
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")

    install(DIRECTORY include/uconfig
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    include(CMakePackageConfigHelpers)

    set(uconfig_VERSION_CONFIG "${PROJECT_BINARY_DIR}/uconfigConfigVersion.cmake")
    set(uconfig_PROJECT_CONFIG "${PROJECT_BINARY_DIR}/uconfigConfig.cmake")
    set(uconfig_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/uconfig/")
    write_basic_package_version_file(${uconfig_VERSION_CONFIG}
        VERSION ${UCONFIG_VERSION_STRING}
        COMPATIBILITY AnyNewerVersion)
    configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/uconfigConfig.cmake.in
        ${uconfig_PROJECT_CONFIG}
        INSTALL_DESTINATION ${uconfig_CONFIG_INSTALL_DIR})


    install(FILES ${uconfig_PROJECT_CONFIG} ${uconfig_VERSION_CONFIG}
        DESTINATION ${uconfig_CONFIG_INSTALL_DIR})
    install(EXPORT uconfigTargets
        DESTINATION ${uconfig_CONFIG_INSTALL_DIR}
        NAMESPACE ${PROJECT_NAME}::)
endif(NOT UCONFIG_BUNDLED_MODE)

set(uconfig_TARGETS_CONFIG "${PROJECT_BINARY_DIR}/uconfigTargets.cmake")
export(EXPORT uconfigTargets
    FILE ${uconfig_TARGETS_CONFIG}
    NAMESPACE ${PROJECT_NAME}::)
