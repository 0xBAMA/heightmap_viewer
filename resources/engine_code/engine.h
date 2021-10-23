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
  GLuint heightmapTexture; // from generated heightmap
  GLuint colormapTexture;  // ground color computed from local slope - maybe do shadows into this, too?

  // compute shader to render
  GLuint renderShader;

  // compute shader to shade colormap
  GLuint shadeShader;

  // for the fullscreen triangle
  GLuint displayShader;
	GLuint displayVAO;
	GLuint displayVBO;

  // initialization
	void createWindow();
  void ImGUIConfigure();
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
