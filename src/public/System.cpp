//
// Created by mKirkpatrick on 1/22/2024.
//

#include <cstdio>
#include "System.h"
#include <SDL.h>

System::System()
{

    // Init SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return;
    }
}

void System::CleanUp()
{

    SDL_Quit();
}
