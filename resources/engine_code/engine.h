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
  float firstPassFrameTimeMs = 0;
  float secondPassFrameTimeMs = 0;
  float erosionPassTimeMs = 0;


	// erosion simulation parameters
	float dt = 1.2; // timestep
	float density = 1.0; // to determine intertia
	float evaporationRate = 0.001;
	float depositionRate = 0.1;
	float minimumVolume = 0.01;
	float friction = 0.05;
  int erosionNumStepsPerFrame = 200;
  bool erosionRunning = true;



  void generateDiamondSquare();
  glm::vec3 surfaceNormal( int x, int y );
  void erode( int steps );
  void prepareMapsFromErosionModel();
  float erosionModel[ 1024 ][ 1024 ] = {{ 0 }};

  // holds the renderer results, before being presented
  GLuint mainRenderTexture; // main view
  GLuint miniRenderTexture; // minimap

  // world representation -
    // - vector of ints on CPU
    // - texture data on GPU

  int worldX;
  int worldY;

  GLuint heightmapTexture; // world height
  std::vector<unsigned char> heightmap;
  void loadMap( int index );
  void sendToGPU();

  GLuint colormapTexture; // world color
  std::vector<unsigned char> colormap;

  // renderer state
  glm::vec2 viewPosition = glm::vec2( 512, 512 );
  int viewerHeight       = 75;
  float viewAngle        = -0.425;
  float maxDistance      = 800.;
  int horizonLine        = 500;
  float heightScalar     = 451.0;
  float offsetScalar     = 0.;
  float fogScalar        = 0.451;
  float stepIncrement    = 0.0;
  float FoVScalar        = 0.85;
  float viewBump         = 80;
  float minimapScalar    = 0.3;
  bool adaptiveHeight    = false;


  // keeps track of when the heightmap has changed and needs to be re-sent
  bool CPU_UpdateFlag = true;


  // compute shaders
  GLuint clearShader;      // to clear
  GLuint shadeShader;     // to shade colormap
  GLuint renderShader;   // to render world
  GLuint minimapShader; // to render minimap

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
