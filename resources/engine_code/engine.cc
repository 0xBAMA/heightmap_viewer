#include "engine.h"
#include "../debug/debug.h"

// This contains the very high level expression of what's going on

engine::engine(){
	programQuitFlag = false;

	createWindow();
	glDebugEnable();
	glSetup();
	ImGUIConfigure();

	while( !programQuitFlag ) {
		drawEverything();
		handleInput();
	}
}

engine::~engine(){
	quit();
}
