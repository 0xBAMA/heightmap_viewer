#include "engine.h"
// This contains the lower level code

void engine::createWindow() {
  if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 ) {
    printf("Error: %s\n", SDL_GetError());
  }

  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER,       1 );
  SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
  SDL_GL_SetAttribute( SDL_GL_RED_SIZE,           8 );
  SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE,         8 );
  SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE,          8 );
  SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE,         8 );

  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE,        24 );
  SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE,       8 );

  SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
  SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 8 );

  cout << "  Creating window....................................";

  auto windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS;
  window = SDL_CreateWindow( windowTitle, 50, 50, totalScreenWidth, totalScreenHeight, windowFlags );

  cout << "done." << endl;

  cout << "  Setting up OpenGL context..........................";
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, 0 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
  GLcontext = SDL_GL_CreateContext( window );

  SDL_GL_MakeCurrent( window, GLcontext );
  SDL_GL_SetSwapInterval( 1 ); // Enable vsync

  if ( gl3wInit() != 0 ) fprintf( stderr, "Failed to initialize OpenGL loader!\n" );

  // graphics API config
  glEnable( GL_DEPTH_TEST );
  glPointSize( 3.0 );
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  // set graphics API clear color and clear window
  clearColor = ImVec4( 0.618, 0.618, 0.618, 1.0 );
  glClearColor( clearColor.x, clearColor.y, clearColor.z, 1.0 );
  glClear( GL_COLOR_BUFFER_BIT );
  SDL_GL_SwapWindow( window );
  SDL_ShowWindow( window );
  cout << "done." << endl;
}

void engine::glSetup() {
  // some info on your current platform
  const GLubyte *renderer = glGetString( GL_RENDERER ); // get renderer string
  const GLubyte *version = glGetString( GL_VERSION );   // version as a string
  printf("    Render Device: %s\n", renderer );
  printf("    OpenGL Support: %s\n\n", version );

  // create the shader for the triangles to cover the screen
  cout << "  Compiling Display Shaders..........................";
  displayShader = Shader( displayVSPath, displayFSPath ).Program;
  cout << "done." << endl;

  cout << "  Setting up VAO, VBO for display geometry...........";
  // based on this, one triangle is significantly faster than two
  // https://michaldrobot.com/2014/04/01/gcn-execution-patterns-in-full-screen-passes/
  std::vector<glm::vec2> points;
  points.push_back( glm::vec2( -1, -1 ) ); // A
  points.push_back( glm::vec2(  3, -1 ) ); // B
  points.push_back( glm::vec2( -1,  3 ) ); // C

  // generate and bind
  glGenVertexArrays( 1, &displayVAO );
  glBindVertexArray( displayVAO );
  glGenBuffers( 1, &displayVBO );
  glBindBuffer( GL_ARRAY_BUFFER, displayVBO );
  cout << "done." << endl;

  // buffer the data
  cout << "  Buffering vertex data..............................";
  glBufferData( GL_ARRAY_BUFFER, sizeof( glm::vec2 ) * points.size(), NULL, GL_STATIC_DRAW );
  glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( glm::vec2 ) * points.size(), &points[0] );
  cout << "done." << endl;

  // set up attributes
  cout << "  Setting up attributes in display shader............";
  GLuint points_attrib = glGetAttribLocation( displayShader, "vertexPosition" );
  glEnableVertexAttribArray( points_attrib );
  glVertexAttribPointer( points_attrib, 2, GL_FLOAT, GL_FALSE, 0, ( 0 ) );
  cout << "done." << endl;


  // create the image textures
  cout << "  Buffering texture data.............................";

  // producing initial image data for the render texture - first, the render texture
  std::vector< unsigned char > imageData( totalScreenWidth * totalScreenHeight * 4, 0 );

  // generate the main render texture
  glGenTextures( 1, &mainRenderTexture );
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, mainRenderTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8UI, totalScreenWidth, totalScreenHeight, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, &imageData[0] );
  glBindImageTexture( 0, mainRenderTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8UI );

  imageData.clear();
  imageData.resize( totalScreenWidth * totalScreenHeight * 4 );
  for( auto it = imageData.begin(); it != imageData.end(); it++ ){
    int index = ( it - imageData.begin() );
    *it = ( unsigned char )(( index / ( totalScreenWidth )) % 256 ) ^ ( unsigned char )(( index % ( 4 * totalScreenWidth )) % 256 );
  }

  // generate the minimap render texture
  glGenTextures( 1, &miniRenderTexture );
  glActiveTexture( GL_TEXTURE1 );
  glBindTexture( GL_TEXTURE_2D, miniRenderTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8UI, totalScreenWidth, totalScreenHeight, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, &imageData[0] );
  glBindImageTexture( 1, miniRenderTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8UI );


  // pick a random heightmap to display initially
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen( rd() ); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution< int > distrib( 1, 30 );

  generateDiamondSquare();
  loadMap( mapPickerItemCurrent );
  erode( 5000 );


  cout << "done." << endl;

  // compute shader compilation
  cout << "  Compiling Compute Shaders..........................";
  clearShader   = CShader(   clearCSPath ).Program;
  renderShader  = CShader(  renderCSPath ).Program;
  minimapShader = CShader( minimapCSPath ).Program;
  shadeShader   = CShader(   shadeCSPath ).Program;
  cout << "done." << endl;
}

void engine::drawEverything() {

  // clear the framebuffer
  glClearColor( clearColor.x, clearColor.y, clearColor.z, 1.0 );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // get the screen dimensions to pass in as uniforms
  ImGuiIO &io = ImGui::GetIO();
  const int screenX = io.DisplaySize.x;
  const int screenY = io.DisplaySize.y;


  if( erosionReady ){
    // this will happen in the main thread, to prevent sync issues w/ multithreaded use of OpenGL
    sendToGPU(); // send the prepared contents to the GPU
    erosionReady = false; // set thread flag to show thread is now running

    // then also invoke the shade shader, to color the heightmap, given the input info
    //    - note that normal vector will be available in the initial colormap
    glUseProgram( shadeShader );  // computing the colormap
    glDispatchCompute( worldX / 16, worldY / 16, 1 );
  }


  // timer query
  GLuint64 startTime, stopTime;
  GLuint queryID[2];
  glGenQueries( 2, queryID );
  glQueryCounter( queryID[0], GL_TIMESTAMP );

  // compute shader to compute the regular display into the corresponding rendertexture
  glUseProgram( renderShader );

  // updating all the uniforms
  glUniform2i( glGetUniformLocation( renderShader, "resolution" ),    screenX, screenY );
  glUniform2f( glGetUniformLocation( renderShader, "viewPosition" ),  viewPosition.x, viewPosition.y );
  glUniform1i( glGetUniformLocation( renderShader, "viewerHeight" ),  viewerHeight );
  glUniform1f( glGetUniformLocation( renderShader, "viewAngle" ),     viewAngle );
  glUniform1f( glGetUniformLocation( renderShader, "maxDistance" ),   maxDistance );
  glUniform1i( glGetUniformLocation( renderShader, "horizonLine" ),   horizonLine );
  glUniform1f( glGetUniformLocation( renderShader, "heightScalar" ),  heightScalar );
  glUniform1f( glGetUniformLocation( renderShader, "offsetScalar" ),  offsetScalar );
  glUniform1f( glGetUniformLocation( renderShader, "fogScalar" ),     fogScalar );
  glUniform1f( glGetUniformLocation( renderShader, "stepIncrement" ), stepIncrement );
  glUniform1f( glGetUniformLocation( renderShader, "FoVScalar" ),     FoVScalar );

  // dispatch to draw into render texture
  glDispatchCompute( std::ceil( totalScreenWidth / 64. ), 1, 1 );
  glQueryCounter( queryID[1], GL_TIMESTAMP );

  GLint timeAvailable = 0;
  while( !timeAvailable )
    glGetQueryObjectiv( queryID[1], GL_QUERY_RESULT_AVAILABLE, &timeAvailable );

  glGetQueryObjectui64v( queryID[0], GL_QUERY_RESULT, &startTime );
  glGetQueryObjectui64v( queryID[1], GL_QUERY_RESULT, &stopTime );
  firstPassFrameTimeMs = ( stopTime - startTime ) / 1000000.;




  // compute shader to compute the minimap into the corresponding rendertexture
  glQueryCounter( queryID[0], GL_TIMESTAMP );
  glUseProgram( minimapShader );

  int heightRef = heightmapReference( glm::ivec2( int( viewPosition.x ), int( viewPosition.y )));
  if ( viewerHeight < heightRef )
    viewerHeight = heightRef + 5;
  int viewerElevation = viewerHeight - heightRef;

  if( !adaptiveHeight )
    viewerElevation = 35;


  // updating all the uniforms
  glUniform2i( glGetUniformLocation( minimapShader, "resolution" ),      screenX / 4, screenY / 3 );
  glUniform2f( glGetUniformLocation( minimapShader, "viewPosition" ),    viewPosition.x, viewPosition.y );
  glUniform1f( glGetUniformLocation( minimapShader, "viewAngle" ),       viewAngle );
  glUniform1f( glGetUniformLocation( minimapShader, "viewBump" ),        viewBump );
  glUniform1f( glGetUniformLocation( minimapShader, "minimapScalar" ),   minimapScalar );
  glUniform1i( glGetUniformLocation( minimapShader, "viewerElevation" ), viewerElevation );

  // dispatch to draw into render texture
  glDispatchCompute( std::ceil( totalScreenWidth / ( 256. ) ), 1, 1 );
  glQueryCounter( queryID[1], GL_TIMESTAMP );

  timeAvailable = 0;
  while( !timeAvailable )
    glGetQueryObjectiv( queryID[1], GL_QUERY_RESULT_AVAILABLE, &timeAvailable );

  glGetQueryObjectui64v( queryID[0], GL_QUERY_RESULT, &startTime );
  glGetQueryObjectui64v( queryID[1], GL_QUERY_RESULT, &stopTime );
  secondPassFrameTimeMs = ( stopTime - startTime ) / 1000000.;

  // make sure all shader invocations have finished before displaying the rendertexture
  glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );



  // present render texture
  glUseProgram( displayShader );
  glBindVertexArray( displayVAO );
  glBindBuffer( GL_ARRAY_BUFFER, displayVBO );
  glUniform1i( glGetUniformLocation( displayShader, "modeSelector" ), 0 );
  glUniform2i( glGetUniformLocation( displayShader, "resolution" ), screenX, screenY );
  glDrawArrays( GL_TRIANGLES, 0, 3 );

  glUniform1i( glGetUniformLocation( displayShader, "modeSelector" ), 1 );
  glUniform2i( glGetUniformLocation( displayShader, "resolution" ), screenX, screenY );
  glDrawArrays( GL_TRIANGLES, 0, 3 );

  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame( window );
  ImGui::NewFrame();

  // show quit confirm window
  quitConf( &quitConfirmFlag );

  // Draw the editor window
  // textEditor();

  // adjustments for the renderer state
  adjustmentWindow();

  // // show the demo window
  // static bool show_demo_window = true;
  // if (show_demo_window)
  //   ImGui::ShowDemoWindow(&show_demo_window);

  // put imgui data into the framebuffer
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

  // swap the double buffers
  SDL_GL_SwapWindow( window );

  // clearing out the render texture for next frame
  glUseProgram( clearShader );
  glDispatchCompute( totalScreenWidth, totalScreenHeight, 1 );
}

void engine::handleInput(){
  SDL_Event event;
  ImGuiIO &io = ImGui::GetIO();

  // handle specific keys
  if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_RightArrow )))
    viewAngle += SDL_GetModState() & KMOD_SHIFT ? 0.05 : 0.005;
  if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_LeftArrow )))
    viewAngle -= SDL_GetModState() & KMOD_SHIFT ? 0.05 : 0.005;

  if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_UpArrow )))
    positionAdjust(SDL_GetModState() & KMOD_SHIFT ?  5. :  1);
  if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_DownArrow )))
    positionAdjust(SDL_GetModState() & KMOD_SHIFT ? -5. : -1);

  if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_PageUp )))
    viewerHeight += SDL_GetModState() & KMOD_SHIFT ? 10 : 1;
  if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_PageDown )))
    viewerHeight -= SDL_GetModState() & KMOD_SHIFT ? 10 : 1;

  if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Home )))
    horizonLine += SDL_GetModState() & KMOD_SHIFT ? 10 : 1;
  if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_Delete )))
    horizonLine -= SDL_GetModState() & KMOD_SHIFT ? 10 : 1;


  while( SDL_PollEvent( &event ) ){
    ImGui_ImplSDL2_ProcessEvent( &event );

    if( event.type == SDL_QUIT )
      programQuitFlag = true;

    if( event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID( window ) )
      programQuitFlag = true;

    if(! io.WantCaptureKeyboard ) // imgui doesn't want the keyboard input
    { // this is used so that keyboard manipulation of widgets doesn't collide with my input handling

      if( ( event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE ) ||
          ( event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_X1 ) )
        quitConfirmFlag = !quitConfirmFlag;

      if( event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE && SDL_GetModState() & KMOD_SHIFT )
        programQuitFlag = true; // shift+escape for force quit, skipping confirmation

    }
  }
}


int engine::heightmapReference( glm::ivec2 p ){
  if( p.x < 0 || p.x > worldX || p.y < 0 || p.y > worldY )
    return 0;

  p.x = std::clamp( p.x, 0, worldX - 1 );
  p.y = std::clamp( p.y, 0, worldY - 1 );
  return int( heightmap[ ( p.x + 1024 * p.y ) * 4 ] );
}

void engine::positionAdjust( float amt ){
  glm::mat2 rotate = glm::mat2( cos( viewAngle ), sin( viewAngle ), -sin( viewAngle ), cos( viewAngle ) );
  glm::vec2 direction = rotate * glm::vec2( 1., 0. );
  viewPosition += amt * direction;
  int heightRef = heightmapReference( glm::ivec2( int( viewPosition.x ), int( viewPosition.y )));
  if ( viewerHeight < heightRef )
    viewerHeight = heightRef + 5;
}

void engine::loadMap( int index ){
  heightmap.clear();
  colormap.clear();

  bool firstTime = true;
  if( firstTime ){ // need to generate textures
    glGenTextures( 1, &heightmapTexture );
    glGenTextures( 1, &colormapTexture );
    firstTime = false;
  }

  unsigned hWidth, hHeight, cWidth, cHeight, error = 0;

  if( index == 31 ){
    prepareMapsFromErosionModel();
  } else {
    std::string heightmapPath = std::string( "maps/map" + std::to_string( index ) + "Height.png" );
    std::string colormapPath  = std::string( "maps/map" + std::to_string( index ) + "Color.png" );

    error = lodepng::decode( heightmap, hWidth, hHeight, heightmapPath.c_str() );
    if( error ) cout << "error loading heightmap data from " << heightmapPath << endl;

    error = lodepng::decode( colormap, cWidth, cHeight, colormapPath.c_str() );
    if( error ) cout << "error loading colormap data from " << colormapPath << endl;

    worldX = hWidth;
    worldY = hHeight;
  }

  // send the updated contents to GPU
  sendToGPU();
}

void engine::sendToGPU(){
  glActiveTexture( GL_TEXTURE2 );
  glBindTexture( GL_TEXTURE_2D, heightmapTexture );
  // glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8UI, worldX, worldY, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, &heightmap[0] );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, worldX, worldY, 0, GL_RGBA, GL_UNSIGNED_BYTE, &heightmap[0] );

  if( !linearTextures ){
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  } else {
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  }

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );

  glBindImageTexture( 2, heightmapTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8UI );

  glActiveTexture( GL_TEXTURE3 );
  glBindTexture( GL_TEXTURE_2D, colormapTexture );
  // glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8UI, worldX, worldY, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, &colormap[0] );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, worldX, worldY, 0, GL_RGBA, GL_UNSIGNED_BYTE, &colormap[0] );

  if( !linearTextures ){
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  } else {
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  }

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );

  glBindImageTexture( 3, colormapTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8UI );
}









void engine::generateDiamondSquare(){
  long unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();

  std::default_random_engine engine{seed};
  std::uniform_real_distribution<float> distribution{0, 1};

  worldX = worldY = 1024;

#ifdef TILE
  constexpr auto size = 1024;
#else
  constexpr auto size = 1025; // for no_wrap
#endif

  constexpr auto edge = size - 1;

  uint16_t map[size][size] = {{0}};
  map[0][0] = map[edge][0] = map[0][edge] = map[edge][edge] = std::numeric_limits<uint16_t>::max()/2;

#ifdef TILE
  heightfield::diamond_square_wrap
#else
  heightfield::diamond_square_no_wrap
#endif
    (size,
    // random
    [&engine, &distribution](float range)
    {
      return distribution(engine) * range;
    },
    // variance
    [](int level) -> float
    {
      return static_cast<float>(std::numeric_limits<uint16_t>::max()/2) * std::pow(0.5f, level);
      // return static_cast<float>(std::numeric_limits<uint16_t>::max()/1.6) * std::pow(0.5f, level);
    },
    // at
    [&map](int x, int y) -> uint16_t&
    {
      return map[y][x];
    }
  );

  // // first pass establishes minimum
  // uint16_t currentMin = std::numeric_limits< uint16_t >::max();
  // uint16_t currentMax = 0;
  // for( int x = 0; x < worldX; x++ )
  //   for( int y = 0; y < worldY; y++ )
  //     currentMin = std::min( currentMin, map[ x ][ y ] ), currentMax = std::max( currentMax, map[ x ][ y ] );
  //
  // // offset by minimum value
  // currentMin++; // make sure there is at least one unit of height at the lowest point
  // currentMax--;
  // for( int x = 0; x < worldX; x++ )
  //   for( int y = 0; y < worldY; y++ )
  //     erosionModel[ x ][ y ] = ( ( 1. ) / ( currentMax - currentMin ) ) * ( map[ x ][ y ] - currentMin );


  for( int x = 0; x < worldX; x++ )
    for( int y = 0; y < worldY; y++ )
      erosionModel[ x ][ y ] = map[ x ][ y ] / static_cast< float >( std::numeric_limits< uint16_t >::max() );
}

glm::vec3 engine::surfaceNormal(int x, int y){
    float scale = 60.0;
    float myPoint = erosionModel[x][y];
    float sqrt2 = sqrt( 2 );

    // cardinal directions
    glm::vec3 n = 0.15f * glm::normalize( glm::vec3( scale * ( myPoint - erosionModel[x + 1][y]), 1.0, 0.0 )); // Positive X
    n += 0.15f * glm::normalize( glm::vec3( scale * ( erosionModel[x - 1][y] - myPoint ), 1.0, 0.0 )); // Negative X
    n += 0.15f * glm::normalize( glm::vec3( 0.0, 1.0, scale * ( myPoint - erosionModel[x][y + 1] )));  // Positive Y
    n += 0.15f * glm::normalize( glm::vec3( 0.0, 1.0, scale * ( erosionModel[x][y - 1] - myPoint )));  // Negative Y

    float pp = erosionModel[x + 1][y + 1];
    float pm = erosionModel[x + 1][y - 1];
    float mp = erosionModel[x - 1][y + 1];
    float mm = erosionModel[x - 1][y - 1];

    // diagonals
    n += 0.1f * glm::normalize( glm::vec3( scale * ( myPoint - pp ) / sqrt2, sqrt2, scale * ( myPoint - pp) / sqrt2));
    n += 0.1f * glm::normalize( glm::vec3( scale * ( myPoint - pm ) / sqrt2, sqrt2, scale * ( myPoint - pm) / sqrt2));
    n += 0.1f * glm::normalize( glm::vec3( scale * ( myPoint - mp ) / sqrt2, sqrt2, scale * ( myPoint - mp) / sqrt2));
    n += 0.1f * glm::normalize( glm::vec3( scale * ( myPoint - mm ) / sqrt2, sqrt2, scale * ( myPoint - mm) / sqrt2));

    return n;
}

struct particle{
	glm::vec2 position;
	glm::vec2 speed = glm::vec2(0.0);

	float volume = 1.0;
	float sedimentFraction = 0.0;
};

void engine::erode( int steps ){
  std::default_random_engine gen;
  std::uniform_int_distribution<int> distX( 0, worldX - 1 );
  std::uniform_int_distribution<int> distY( 0, worldY - 1 );

  // run the simulation for the specified number of steps
  for( int i = 0; i < steps; i++ ){
    //spawn a new particle at a random position
    particle p;
    p.position = glm::vec2( distX( gen ), distY( gen ) );

    //while the droplet exists (drop volume > 0)
    while( p.volume > 0 ){
      glm::ivec2 initialPosition = p.position;    // cache the initial position
      glm::vec3 normal = surfaceNormal( initialPosition.x, initialPosition.y );

      // newton's second law to calculate acceleration
      p.speed += dt * glm::vec2( normal.x, normal.z ) / ( p.volume * density ); // F = MA, so A = F/M

      // update position based on new speed
      p.position += dt * p.speed;

      // friction factor to attenuate speed
      p.speed *= ( 1.0 - dt * friction );

      // discard the droplet if it has gone out of bounds
      if( !glm::all( glm::greaterThanEqual( p.position, glm::vec2( 0 ) ) ) ||
          !glm::all( glm::lessThan( glm::ivec2( p.position ), glm::ivec2( worldX, worldY ) ) ) ) break;

      // sediment capacity
      glm::ivec2 ref = glm::ivec2( p.position.x, p.position.y );
      float maxSediment = p.volume * glm::length( p.speed ) * ( erosionModel[ initialPosition.x ][ initialPosition.y ] - erosionModel[ ref.x ][ ref.y ] );
      maxSediment = std::max( maxSediment, 0.0f ); // don't want negative values here
      float sedimentDiff = maxSediment - p.sedimentFraction;

      // update sediment content, deposit on the heightmap
      p.sedimentFraction += dt * depositionRate * sedimentDiff;
      erosionModel[ initialPosition.x ][ initialPosition.y ] -= dt * p.volume * depositionRate * sedimentDiff;

      p.volume *= ( 1.0 - dt * evaporationRate ); // evaporate some of the droplet
    }
  }
}


void engine::prepareMapsFromErosionModel(){
  // goal here is to go from the floating point heightmap model to the four channel unsigned char
  // array, as required by the GPU - heightmap is grayscale, colormap is initialized with normals

  // once this is completed, sendToGPU can be called, to send the heightmap and colormap arrays

  heightmap.resize(worldX * worldY * 4);
  colormap.resize( worldX * worldY * 4);

  for(int x = 0; x < worldX; x++){
    for(int y = 0; y < worldY; y++){

      int index = ( x + y * worldX ) * 4;
      unsigned char modelRef = static_cast< unsigned char >( erosionModel[ x ][ y ] * 255 );
      glm::vec3    normalRef = surfaceNormal( x, y ); // put x,y,z into colormap

      // cout << "modelRef " << x << " " << y << " " << int(modelRef) << endl;

      heightmap[ index + 0 ] = modelRef;
      heightmap[ index + 1 ] = modelRef;
      heightmap[ index + 2 ] = modelRef;
      heightmap[ index + 3 ] = 255;

      // cout << "vector is " << glm::length( normalRef ) << " units long" << endl;
      normalRef.y *= 0.001;
      normalRef = glm::normalize( normalRef );

      colormap[ index + 0 ] = static_cast< unsigned char >( ( ( normalRef.x + 1. ) / 2.) * 255 );
      colormap[ index + 1 ] = static_cast< unsigned char >( ( ( normalRef.y + 1. ) / 2.) * 255 );
      colormap[ index + 2 ] = static_cast< unsigned char >( ( ( normalRef.z + 1. ) / 2.) * 255 );
      colormap[ index + 3 ] = 255;
    }
  }
}

void engine::quit() {
  cout << "  Shutting down...................................";

  // tell the thread it should stop, before calling the rest of the shutdown - gives it some time
  threadShouldRun = false;

  // shutdown everything
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  // destroy window
  SDL_GL_DeleteContext( GLcontext );
  SDL_DestroyWindow( window );
  SDL_Quit();

  // join the worker thread
    // this is a blocking operation, so it will hang while waiting for the final loop iteration
  erosionThread.join();

  // done
  cout << "goodbye." << endl;
}
