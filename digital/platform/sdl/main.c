#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "platform.h"

int main(int argc, char *argv[])
{
    platform_init(); // Window, renderer, audio stream
    core_init(); // Game state
    while (core_frame()); // Tick until quit

    SDL_Quit();
    return 0;
}
