#ifndef ENGINE
#define ENGINE

#include "includes.h"

class engine{
public:
	engine();
	~engine();

private:
  int totalScreenWidth  =  WIDTH - 100;
  int totalScreenHeight = HEIGHT - 100;

	SDL_Window * window;
	SDL_GLContext GLcontext;
  ImVec4 clearColor;

  // holds the renderer result
  GLuint renderTexture;

  // holds the heightmap data - initially load from disk
  GLuint heightmapTexture;   // from generated heightmap
  GLuint colormapTexture;  // ground color computed from local slope - maybe do shadows into this, too?

  // compute shaders
  GLuint renderShader; // to render
  GLuint shadeShader; // to shade colormap

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
  void handleInput();
  void textEditor();

  // quitting
	void quit();
	void quitConf(bool *open);

  // control flags
	bool quitConfirmFlag = false;
	bool programQuitFlag = false;
};

#endif
