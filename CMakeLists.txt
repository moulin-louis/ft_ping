cmake_minimum_required(VERSION 3.22)

set(CMAKE_C_COMPILER /usr/bin/clang)
project(ft_ping C)

add_subdirectory(src/libft)
add_compile_options(-Wall -Werror -Wextra)
include_directories(./inc src/libft/inc)
add_executable(ft_ping src/main.c
        inc/ft_ping.h
        src/echo.c)
target_link_libraries(ft_ping ft)