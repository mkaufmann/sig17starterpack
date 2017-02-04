cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(reference VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 14)


add_library(baseline
    src/ngramer.cpp)
target_include_directories(baseline PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)


add_executable(runner
    src/runner.cpp)
target_include_directories(runner PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)
target_link_libraries(runner baseline)

enable_testing()
find_package(GTest REQUIRED)
add_executable(reference_tester src/tester.cpp)
target_link_libraries(reference_tester baseline GTest::GTest GTest::Main)
GTEST_ADD_TESTS(reference_tester "" AUTO)