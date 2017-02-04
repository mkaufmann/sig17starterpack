cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(datagen VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 14)

add_executable(generator
    src/generator.cpp)
target_include_directories(generator PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)
target_link_libraries(generator baseline)

add_executable(bencher
    src/popenRWE.cpp
    src/bencher.cpp)
target_include_directories(bencher PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)
