cmake_minimum_required(VERSION 3.8...3.17)
project(composite_message C CXX)

set(CMAKE_C_STANDARD 11)

option(BUILD_TESTS "Build CompositeMessage tests" ON)

add_subdirectory(composite-message)

if (BUILD_TESTS)
    add_subdirectory(lib/catch2)
    include(CTest)
    add_subdirectory(tests)
    add_test(CliTests tests/composite_message_tests)
endif ()
