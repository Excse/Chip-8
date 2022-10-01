# https://github.com/selyunin/gtest_submodule/blob/master/CMakeLists.txt

cmake_minimum_required(VERSION 3.23)

########################################
# Project setup
########################################
set(PROJECT_NAME chip8_emulator)
project(${PROJECT_NAME})

########################################
# Set up the compiler flags
########################################
set(CMAKE_CXX_FLAGS "-pedantic -Wall -Werror -Wshadow -std=c++23")
set(CMAKE_CXX_STANDARD 23)

########################################
# Define include directories
########################################
set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)
include_directories(${COMMON_INCLUDES})

########################################
# Source files
########################################
set(SRC_FILES
        ${PROJECT_SOURCE_DIR}/src/main.cpp
        ${PROJECT_SOURCE_DIR}/src/chip8.cpp)

########################################
# Add other libraries
########################################
include(FindPkgConfig)

pkg_search_module(SDL2 REQUIRED sdl2)
include_directories(${SDL2_INCLUDE_DIRS})

add_subdirectory(./libs/cxxopts)
include_directories(${cxxopts_SOURCE_DIR}/include)

########################################
# Compile source files into a library
########################################
add_library(${PROJECT_NAME}_lib ${SRC_FILES})

########################################
# Main is separate (e.g. library client)
########################################
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})

########################################
# Linking the main against the library
########################################
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib)


########################################
# Testing
########################################
add_subdirectory(./libs/googletest)

enable_testing()

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

########################################
# Test files
########################################
file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)

########################################
# Unit Tests
########################################
add_executable(${PROJECT_NAME}_tests ${TEST_SRC_FILES})

########################################
# Linking the main against the library
########################################
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib)

########################################
# Standard linking to gtest stuff.
########################################
target_link_libraries(${PROJECT_NAME}_tests gtest gtest_main)

########################################
# Extra linking for the project.
########################################
target_link_libraries(${PROJECT_NAME}_tests ${PROJECT_NAME}_lib ${SDL2_LIBRARIES})

add_test(UnitTests ${PROJECT_NAME}_tests)
