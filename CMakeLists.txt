cmake_minimum_required(VERSION 3.0.0)

project(Hair_Renderer VERSION 0.0.1)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(PCL REQUIRED)

file(GLOB ENGINE_SOURCES
"src/engine/*.cpp"
"src/engine/*.h"
"deps/imgui/*.cpp"
"deps/imgui/*.h"
)

add_library(Engine ${ENGINE_SOURCES})
target_link_libraries(Engine glfw GLEW::GLEW ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} -lGL  ${PCL_LIBRARIES})
target_include_directories(Engine PUBLIC ${CMAKE_SOURCE_DIR}/deps ${CMAKE_SOURCE_DIR}/deps/imgui ${PCL_INCLUDE_DIRS})
add_definitions(${PCL_DEFINITIONS})

file(GLOB APP_SOURCES
"src/*.cpp"
"src/*.h"
)

add_executable(HairViewer ${APP_SOURCES})
target_link_libraries(HairViewer PRIVATE Engine)


add_executable(LUTGenerator 
"src/marschner/main.cpp" 
"src/marschner/generator.hpp")
# target_link_libraries(LUTGenerator PRIVATE Engine)
target_include_directories(LUTGenerator PUBLIC ${CMAKE_SOURCE_DIR}/deps)


set_target_properties(HairViewer PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug
)
set_target_properties(LUTGenerator PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug
)

target_compile_definitions(LUTGenerator PUBLIC SHADERS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/")


