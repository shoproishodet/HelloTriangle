cmake_minimum_required(VERSION 3.24)

set(PROJECT_NAME HelloTriangle)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/Include)

add_subdirectory(GLM)
add_subdirectory(Source)


