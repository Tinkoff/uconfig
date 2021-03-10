cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
set(CMAKE_MACOSX_RPATH 1)

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

add_executable(variable variable.cpp)
add_executable(vector vector.cpp)
add_executable(env env.cpp)
add_executable(rapidjson rapidjson.cpp)
add_executable(config_vars config_vars.cpp)
add_executable(config_vector config_vector.cpp)
add_executable(config_nested config_nested.cpp)

target_link_libraries(variable uconfig::uconfig gtest_main)
target_link_libraries(vector uconfig::uconfig gtest_main)
target_link_libraries(env uconfig::uconfig gtest_main)
target_link_libraries(rapidjson uconfig::uconfig gtest_main)
target_link_libraries(config_vars uconfig::uconfig gtest_main)
target_link_libraries(config_vector uconfig::uconfig gtest_main)
target_link_libraries(config_nested uconfig::uconfig gtest_main)

add_test(NAME variable COMMAND variable)
add_test(NAME vector COMMAND vector)
add_test(NAME env COMMAND env)
add_test(NAME rapidjson COMMAND rapidjson)
add_test(NAME config_vars COMMAND config_vars)
add_test(NAME config_vector COMMAND config_vector)
add_test(NAME config_nested COMMAND config_nested)
