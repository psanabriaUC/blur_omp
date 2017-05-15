cmake_minimum_required(VERSION 3.0)
project(blur_omp C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_EXTENSIONS OFF)

find_package(PNG REQUIRED)
find_package(OpenMP REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")

add_executable(blur_omp main.c image_utils.h image_utils.c)
target_compile_options(blur_omp PUBLIC -Wall -pedantic -Wextra -Werror)
target_include_directories(blur_omp SYSTEM PUBLIC ${PNG_INCLUDE_DIRS})
target_link_libraries(blur_omp ${PNG_LIBRARIES})
