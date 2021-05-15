cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
set(CMAKE_MACOSX_RPATH 1)

if(NOT TARGET testing)
    set(TETING_TARGET_LOCAL "True")
    add_custom_target(testing COMMAND ${CMAKE_CTEST_COMMAND})
else()
    set(TETING_TARGET_LOCAL "False")
endif()

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

function(add_unit_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} ${PROJECT_NAME}::${PROJECT_NAME} gtest_main)
    add_test(NAME ${name} COMMAND $<TARGET_FILE:${name}>)
    if (TETING_TARGET_LOCAL)
        add_dependencies(testing ${name})
    endif ()
endfunction()

add_unit_test(variable variable.cpp)
add_unit_test(vector vector.cpp)
add_unit_test(env env.cpp)
add_unit_test(rapidjson rapidjson.cpp)
add_unit_test(config_vars config_vars.cpp)
add_unit_test(config_vector config_vector.cpp)
add_unit_test(config_nested config_nested.cpp)
