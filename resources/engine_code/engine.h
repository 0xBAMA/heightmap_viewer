#ifndef ENGINE
#define ENGINE

#include "includes.h"

class engine
{
public:
	engine();
	~engine();

private:
  int totalScreenWidth  =  WIDTH;
  int totalScreenHeight = HEIGHT;

	SDL_Window * window;
	SDL_GLContext GLcontext;
  ImVec4 clearColor;

  // holds the renderer result
  GLuint renderTexture;

  // holds the heightmap data
  GLuint heightmapTexture;
  GLuint colormapTexture;

  // for the fullscreen triangle
  GLuint displayShader;
	GLuint displayVAO;
	GLuint displayVBO;

  // main loop support
	void createWindow();
	void glSetup();

  // main loop functions
	void drawEverything();

  // quitting
	void quit();
	void quitConf(bool *open);

  // control flags
	bool quitConfirmFlag = false;
	bool programQuitFlag = false;
};

#endif
