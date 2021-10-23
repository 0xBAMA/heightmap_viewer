#include "engine.h"
#include "../debug/debug.h"

// This contains the very high level expression of what's going on

engine::engine()
{
    programQuitFlag = false;

    createWindow();
    gl_debug_enable();
    glSetup();

    while(!programQuitFlag) // main loop
    {
        drawEverything();
    }
}

engine::~engine()
{
    quit();
}
