cmake_minimum_required(VERSION 3.21)
project(gfbaf C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)

find_package(SDL3 REQUIRED CONFIG)

# Shared game core, not linked to SDL3
file(GLOB CORE_SOURCES CONFIGURE_DEPENDS core/src/*.c)
add_library(gfbaf_core STATIC ${CORE_SOURCES})
target_include_directories(gfbaf_core PUBLIC core/include)

if(MSVC)
    target_compile_options(gfbaf_core PRIVATE /W4 /WX)
else()
    target_compile_options(gfbaf_core PRIVATE
      -Wall -Wextra -Werror -Wconversion -Wsign-conversion)
endif()

# SDL3 platform layer to emulate PIC display, inputs, ticks, and audio
file(GLOB SDL_SOURCES CONFIGURE_DEPENDS platform/sdl/*.c)
add_executable(gfbaf_sdl ${SDL_SOURCES})
target_link_libraries(gfbaf_sdl PRIVATE gfbaf_core SDL3::SDL3)

# Copy SDL3.dll next to .exe so it runs from build folder
if(WIN32)
    add_custom_command(TARGET gfbaf_sdl POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:gfbaf_sdl>
    COMMAND_EXPAND_LISTS)
endif()
