cmake_minimum_required(VERSION 3.16)
project(schedlab C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Strict warnings, as errors. -O2 is explicit so every build type measures
# the same code.
add_compile_options(-Wall -Wextra -Werror -O2)

# One platform file per OS. Nothing else in the code base is #ifdef'd.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(PLATFORM_SRC src/platform_linux.c)
else()
  message(FATAL_ERROR "No platform layer for ${CMAKE_SYSTEM_NAME} yet (QNX comes in milestone 6)")
endif()

find_package(Threads REQUIRED)

add_executable(schedlab
  src/main.c
  ${PLATFORM_SRC}
  src/generated/cmdline.c)

# POSIX.1-2008 features (clock_nanosleep, pthread barriers, ...) under -std=c11.
target_compile_definitions(schedlab PRIVATE _POSIX_C_SOURCE=200809L)
target_include_directories(schedlab PRIVATE include src/generated)
target_link_libraries(schedlab PRIVATE Threads::Threads)
