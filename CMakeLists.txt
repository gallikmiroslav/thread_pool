cmake_minimum_required(VERSION 3.21)
project(thread_pool C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_VERBOSE_MAKEFILE ON)


add_executable(thread_pool test.c thread_pool.c thread_pool.h)

target_compile_options(thread_pool PRIVATE
        -Wall                    # Enable all common warnings
        -Werror                  # Treat warnings as errors
)

target_link_libraries(thread_pool pthread)