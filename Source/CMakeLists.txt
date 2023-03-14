cmake_minimum_required(VERSION 3.24)

find_package(Vulkan REQUIRED)

set(GLFW_ROOT_DIRS "C:/Program Files (x86)/glfw")
set(GLFW_INCLUDE_DIRS ${GLFW_ROOT_DIRS}/include)
set(GLFW_LIB_DIRS ${GLFW_ROOT_DIRS}/lib)
set(GLFW_LIB ${GLFW_LIB_DIRS}/libglfw3.a)

set(SOURCES
        main.cpp
        Application.cpp
        Externs.cpp
        QueueFamilyIndices.cpp
        SwapChainSupportDetails.cpp
        )

set(INCLUDES
        ${HEADERS}/QueueFamilyIndices.h
        ${HEADERS}/Application.h
        ${HEADERS}/Externs.h
        ${HEADERS}/SwapChainSupportDetails.h
        )

add_executable(${PROJECT_NAME}
        ${SOURCES}
        ${INCLUDES}
        )

add_executable(Helper helper.cpp)

target_include_directories(${PROJECT_NAME}
        PUBLIC ${VULKAN_INCLUDE_DIR}
        PUBLIC ${GLFW_INCLUDE_DIRS}
        PUBLIC ${HEADERS}
        GLM
        )

target_link_libraries(${PROJECT_NAME}
        Vulkan::Vulkan
        ${GLFW_LIB}
        )

target_include_directories(Helper
        PUBLIC ${VULKAN_INCLUDE_DIR}
        PUBLIC ${GLFW_INCLUDE_DIRS}
        PUBLIC ${HEADERS}
        GLM
        )

target_link_libraries(Helper
        Vulkan::Vulkan
        ${GLFW_LIB}
        )