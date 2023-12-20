cmake_minimum_required(VERSION 3.0.0)

project(Hair_Renderer VERSION 0.0.1)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

file(GLOB SOURCES
"src/*.cpp"
"src/*.h"
"src/engine/*.cpp"
"src/engine/*.h"
)


add_executable(HairViewer ${SOURCES})

target_link_libraries(HairViewer libglfw3.a GLEW::GLEW ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} -lGL)
target_include_directories(HairViewer PUBLIC ${CMAKE_SOURCE_DIR}/deps)


set_target_properties(HairViewer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug
)