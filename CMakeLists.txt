cmake_minimum_required(VERSION 3.10)

project(VulkanLearn)

file(GLOB SRC_FILES
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.hpp"
    "${PROJECT_SOURCE_DIR}/src/*.c"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.cc"
)

find_package(Vulkan REQUIRED)

if(Vulkan_FOUND)
    include_directories(${Vulkan_INCLUDE_DIRS})
endif()

set(GLFW_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/lib/glfw/include)
set(GLM_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/lib/glm)

file(GLOB GLFW_LIBRARIES "${CMAKE_SOURCE_DIR}/lib/glfw/lib-mingw-w64/*")

include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME} ${GLFW_LIBRARIES})

target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_20)

if(Vulkan_FOUND)
    target_link_libraries(${CMAKE_PROJECT_NAME} ${Vulkan_LIBRARIES})
endif()

add_custom_command(TARGET ${CMAKE_PROJECT_NAME}
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME})

add_custom_target(run
                  DEPENDS ${CMAKE_PROJECT_NAME}
                  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME})