#include "engine.h"
// This contains the lower level code

void engine::createWindow() {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    printf("Error: %s\n", SDL_GetError());
  }

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

  cout << "  Creating window....................................";

  auto windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS;
  window = SDL_CreateWindow( windowTitle, 50, 50, totalScreenWidth, totalScreenHeight, windowFlags);

  cout << "done." << endl;

  cout << "  Setting up OpenGL context..........................";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  GLcontext = SDL_GL_CreateContext(window);

  SDL_GL_MakeCurrent(window, GLcontext);
  SDL_GL_SetSwapInterval(1); // Enable vsync

  if ( gl3wInit() != 0 ) fprintf(stderr, "Failed to initialize OpenGL loader!\n");

  // graphics API config
  glEnable(GL_DEPTH_TEST);
  glPointSize(3.0);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // set graphics API clear color and clear window
  glClearColor( 0.618, 0.618, 0.618, 1.0 );
  glClear( GL_COLOR_BUFFER_BIT );
  SDL_GL_SwapWindow( window );
  SDL_ShowWindow( window );
  cout << "done." << endl;
}

void engine::glSetup() {
  // some info on your current platform
  const GLubyte *renderer = glGetString(GL_RENDERER); // get renderer string
  const GLubyte *version = glGetString(GL_VERSION);   // version as a string
  printf("    Render Device: %s\n", renderer);
  printf("    OpenGL Support: %s\n\n", version);

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
  glBufferData( GL_ARRAY_BUFFER, sizeof(glm::vec2) * points.size(), NULL, GL_STATIC_DRAW);
  glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * points.size(), &points[0]);
  cout << "done." << endl;

  // set up attributes
  cout << "  Setting up attributes in display shader............";
  GLuint points_attrib = glGetAttribLocation(displayShader, "vertexPosition");
  glEnableVertexAttribArray(points_attrib);
  glVertexAttribPointer(points_attrib, 2, GL_FLOAT, GL_FALSE, 0, (0));
  cout << "done." << endl;


  // create the image textures
  cout << "  Buffering texture data.............................";

  // producing initial image data for the render texture - first, the render texture
  std::vector<unsigned char> imageData;
  imageData.resize( totalScreenWidth * totalScreenHeight * 4 );
  for( auto it = imageData.begin(); it != imageData.end(); it++ ){
    int index = ( it - imageData.begin() );
    *it = ( unsigned char )(( index / ( totalScreenWidth )) % 256 ) ^ ( unsigned char )(( index % ( 4 * totalScreenWidth )) % 256 );
  }

  // generate the render texture
  glGenTextures( 1, &renderTexture );
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_RECTANGLE, renderTexture );

  // send it
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_RECTANGLE, renderTexture );
  glTexImage2D( GL_TEXTURE_RECTANGLE, 0, GL_RGBA8UI, totalScreenWidth, totalScreenHeight, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, &imageData[0] );
  glBindImageTexture( 0, renderTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8UI );


  // next, the initial heightmap data
  unsigned hWidth, hHeight, cWidth, cHeight, error = 0;
  error = lodepng::decode( heightmap, hWidth, hHeight, heightmapPath );
  if( error ) cout << "error loading heightmap data" << endl;

  // send to GPU
  glGenTextures( 1, &heightmapTexture );
  glActiveTexture( GL_TEXTURE1 );
  glBindTexture( GL_TEXTURE_RECTANGLE, heightmapTexture );
  glTexImage2D( GL_TEXTURE_RECTANGLE, 0, GL_RGBA8UI, hWidth, hHeight, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, &heightmap[0] );
  glBindImageTexture( 1, heightmapTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8UI );


  // finally, the initial colormap data
  error = lodepng::decode( colormap, cWidth, cHeight, colormapPath );
  if( error ) cout << "error loading colormap data" << endl;

  glGenTextures( 1, &colormapTexture );
  glActiveTexture( GL_TEXTURE2 );
  glBindTexture( GL_TEXTURE_RECTANGLE, colormapTexture );
  glTexImage2D( GL_TEXTURE_RECTANGLE, 0, GL_RGBA8UI, cWidth, cHeight, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, &colormap[0] );
  glBindImageTexture( 2, colormapTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8UI );

  cout << "done." << endl;

  // compute shader compilation
  cout << "  Compiling Compute Shaders..........................";
  clearShader  = CShader(  clearCSPath ).Program;
  renderShader = CShader( renderCSPath ).Program;
  shadeShader  = CShader(  shadeCSPath ).Program;
  cout << "done." << endl;
}

void engine::drawEverything() {

  // clear the framebuffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // get the screen dimensions to pass in as uniforms
  ImGuiIO &io = ImGui::GetIO();
  const int screenX = io.DisplaySize.x;
  const int screenY = io.DisplaySize.y;



  // compute shader to compute the colormap
  //  ...



  // compute shader prepares the render texture
  glUseProgram( renderShader );

  // updating all the uniforms
  glUniform2i( glGetUniformLocation( renderShader, "resolution" ),   screenX, screenY );
  glUniform2f( glGetUniformLocation( renderShader, "viewPosition" ), viewPosition.x, viewPosition.y );
  glUniform1i( glGetUniformLocation( renderShader, "viewerHeight"),  viewerHeight );
  glUniform1f( glGetUniformLocation( renderShader, "viewAngle"),     viewAngle );
  glUniform1f( glGetUniformLocation( renderShader, "maxDistance"),   maxDistance );
  glUniform1i( glGetUniformLocation( renderShader, "horizonLine"),   horizonLine );
  glUniform1f( glGetUniformLocation( renderShader, "heightScalar"),  heightScalar );
  glUniform1f( glGetUniformLocation( renderShader, "fogScalar"),     fogScalar );
  glUniform1f( glGetUniformLocation( renderShader, "stepIncrement"), stepIncrement );
  glUniform1f( glGetUniformLocation( renderShader, "FoVScalar"),     FoVScalar );

  // dispatch to draw into render texture
  glDispatchCompute( std::ceil( totalScreenWidth / 64. ), 1, 1 );

  glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );


  // present render texture
  glUseProgram( displayShader );
  glBindVertexArray( displayVAO );
  glBindBuffer( GL_ARRAY_BUFFER, displayVBO );
  glUniform2i( glGetUniformLocation( displayShader, "resolution" ), screenX, screenY );
  glDrawArrays( GL_TRIANGLES, 0, 3 );

  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame(window);
  ImGui::NewFrame();

  // show quit confirm window
  quitConf(&quitConfirmFlag);

  // Draw the editor window
  textEditor();

  // adjustments for the renderer state
  adjustmentWindow();

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
  if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_RightArrow ))) viewAngle += SDL_GetModState() & KMOD_SHIFT ? 0.05 : 0.005;
  if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_LeftArrow )))  viewAngle -= SDL_GetModState() & KMOD_SHIFT ? 0.05 : 0.005;

  if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_UpArrow )))    positionAdjust(SDL_GetModState() & KMOD_SHIFT ?  1. :  0.2);
  if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_DownArrow )))  positionAdjust(SDL_GetModState() & KMOD_SHIFT ? -1. : -0.2);

  if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_PageUp )))     viewerHeight += SDL_GetModState() & KMOD_SHIFT ? 10 : 1;
  if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_PageDown )))   viewerHeight -= SDL_GetModState() & KMOD_SHIFT ? 10 : 1;



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


int engine::heightmapReference(glm::ivec2 p){
  p.x = std::clamp(p.x, 0, 1023);
  p.y = std::clamp(p.y, 0, 1023);
  return int( heightmap[ ( p.x + 1024 * p.y ) * 4 ] );
}

void engine::positionAdjust(float amt){
  glm::mat2 rotate = glm::mat2(cos( viewAngle ), sin( viewAngle ), -sin( viewAngle ), cos( viewAngle ));
  glm::vec2 direction = rotate * glm::vec2( 1., 0. );
  viewPosition += amt * direction;
  viewerHeight = std::max(viewerHeight * heightScalar, heightmapReference(glm::ivec2(int(viewPosition.x), int(viewPosition.y))) * heightScalar);
  viewerHeight /= heightScalar;
}


void engine::quit() {
  cout << "  Shutting down...................................";

  // shutdown everything
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  // destroy window
  SDL_GL_DeleteContext( GLcontext );
  SDL_DestroyWindow( window );
  SDL_Quit();

  cout << "goodbye." << endl;
}
