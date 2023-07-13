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
	int erosionNumStepsPerFrame = 4000;



	// thread for the erosion calcs
	bool threadShouldRun = true;
	bool erosionReady = true;
	std::thread erosionThread{ [=] () {
		while ( threadShouldRun ) {
			if ( erosionReady || mapPickerItemCurrent != 31 ) {
				std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
			} else {
				// timer start
				auto tstart = std::chrono::high_resolution_clock::now();

				int count = erosionNumStepsPerFrame;
				// erosion mode active, do the work
				while ( ( count -= 500 ) > 0 && threadShouldRun )
					erode( 500 );

				prepareMapsFromErosionModel();

				// timer end
				erosionPassTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now()-tstart ).count();

				erosionReady = true;
			}
		}
	}};

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
	int horizonLine        = 700;
	float heightScalar     = 1200.0;
	float offsetScalar     = 0.;
	float fogScalar        = 0.451;
	float stepIncrement    = 0.0;
	float FoVScalar        = 0.85;
	float viewBump         = 275;
	float minimapScalar    = 0.3;
	bool adaptiveHeight    = false;

	int mapPickerItemCurrent = 31;
	int linearTextures = 0;

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
