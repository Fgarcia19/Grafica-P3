cmake_minimum_required(VERSION 3.25)
project(E2_Iluminacion)

set(CMAKE_CXX_STANDARD 17)
include_directories(./include/)
set(CMAKE_CXX_FLAGS -I/opt/homebrew/include)
link_directories(/opt/homebrew/lib)

add_executable(E2_Iluminacion 2.1.basic_lighting_diffuse/basic_lighting_diffuse.cpp include/glad.c)
target_link_libraries(E2_Iluminacion glfw.3)