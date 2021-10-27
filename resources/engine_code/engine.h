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
  float prevFrameTimeMs = 0;

  // holds the renderer result, before being presented
  GLuint renderTexture;

  // world representation -
    // - vector of ints on CPU
    // - texture data on GPU

  GLuint heightmapTexture; // world height
  std::vector<unsigned char> heightmap;
  void loadMap( int index );

  GLuint colormapTexture; // world color
  std::vector<unsigned char> colormap;

  // renderer state
  glm::vec2 viewPosition = glm::vec2( -950, 1245 );
  int viewerHeight       = 1575;
  float viewAngle        = -0.425;
  float maxDistance      = 3000.;
  int horizonLine        = 1486;
  float heightScalar     = 900.0;
  float offsetScalar     = 300.;
  float fogScalar        = 0.017;
  float stepIncrement    = 0.0;
  float FoVScalar        = 0.275;
  // float FoVScalar        = 3.14159265 / 4.;


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
