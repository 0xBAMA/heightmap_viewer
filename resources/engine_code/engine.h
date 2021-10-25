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

  // renderer state
  glm::vec2 viewPosition = glm::vec2( 512, 512 );
  int viewerHeight       = 50;
  float viewAngle        = 0.5;
  float maxDistance      = 300.;
  int horizonLine        = 120;
  float heightScalar     = 1.0;
  float fogScalar        = 22.0;
  float stepIncrement    = 0.005;
  float FoVScalar        = 3.14159265 / 4.;


  // keeps track of when the heightmap has changed and needs to be re-sent
  bool CPU_UpdateFlag = true;


  // compute shaders
  GLuint clearShader;   // to clear
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
  void adjustmentWindow();
  void positionAdjust(float amt);
  int heightmapReference(glm::ivec2 p);

  // quitting
	void quit();
	void quitConf(bool *open);

  // control flags
	bool quitConfirmFlag = false;
	bool programQuitFlag = false;
};

#endif
