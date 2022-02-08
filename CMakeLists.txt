cmake_minimum_required(VERSION 3.18)
project(minewall)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++17-extensions")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-incompatible-library-redeclaration")
if (CMAKE_VERSION VERSION_LESS 3.0.0)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
else()
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(ModLoader REQUIRED)

add_library(minewall SHARED main.cpp stub.c)
set_target_properties(minewall PROPERTIES COMPILE_FLAGS "-S")
add_custom_command(TARGET minewall PRE_LINK COMMAND node ${CMAKE_SOURCE_DIR}/preprocessor.js "\"$<TARGET_OBJECTS:minewall>\"")
target_link_libraries(minewall PUBLIC ModLoader)


