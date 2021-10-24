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

  // holds the renderer result, before being presented
  GLuint renderTexture;

  // world representation -
    // - vector of ints on CPU
    // - texture data on GPU

  GLuint heightmapTexture; // world height
  std::vector<unsigned char> heightmap;

  GLuint colormapTexture; // world color
  std::vector<unsigned char> colormap;


  // keeps track of when the heightmap has changed and needs to be re-sent
  bool CPU_UpdateFlag = true;


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
