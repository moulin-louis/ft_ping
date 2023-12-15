cmake_minimum_required(VERSION 3.22)
project(ft C)

include_directories(inc)
add_compile_options(-Wall -Werror -Wextra -O3 -mavx -msse2 -march=native -fno-builtin -pedantic)

add_library(ft STATIC src/ft_atoi.c
        src/ft_calloc.c
        src/ft_isascii.c
        src/ft_bzero.c
        src/ft_isalnum.c
        src/ft_isalpha.c
        src/ft_isdigit.c
        src/ft_isprint.c
        src/ft_memchr.c
        src/ft_memcmp.c
        src/ft_memcpy.c
        src/ft_strdup.c
        src/ft_memmove.c
        src/ft_memset.c
        src/ft_strchr.c
        src/ft_strlcat.c
        src/ft_strlcpy.c
        src/ft_strlen.c
        src/ft_strncmp.c
        src/ft_strnstr.c
        src/ft_strrchr.c
        src/ft_tolower.c
        src/ft_toupper.c
        src/ft_substr.c
        src/ft_strjoin.c
        src/ft_strtrim.c
        src/ft_split.c
        src/ft_itoa.c
        src/ft_strmapi.c
        src/ft_striteri.c
        src/ft_putchar_fd.c
        src/ft_putstr_fd.c
        src/ft_putendl_fd.c
        src/ft_putnbr_fd.c
        src/ft_lstadd_back.c
        src/ft_lstadd_front.c
        src/ft_lstclear.c
        src/ft_lstdelone.c
        src/ft_lstiter.c
        src/ft_lstlast.c
        src/ft_lstmap.c
        src/ft_lstnew.c
        src/ft_lstsize.c
        src/clean_split.c
        src/ft_lstfilter.c
        src/ft_lstsort.c
        src/hexdump.c)
