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

  // first, the render texture
  glGenTextures(1, &renderTexture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_RECTANGLE, renderTexture);

  // texture parameters
  glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // producing initial image data for the render texture
  std::vector<unsigned char> imageData;
  imageData.resize( totalScreenWidth * totalScreenHeight * 4 );
  for( auto it = imageData.begin(); it != imageData.end(); it++ ){
    int index = ( it - imageData.begin() );
    *it = ( unsigned char )(( index / ( totalScreenWidth )) % 256 ) ^ ( unsigned char )(( index % ( 4 * totalScreenWidth )) % 256 );
  }

  // send it
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_RECTANGLE, renderTexture );
  glTexImage2D( GL_TEXTURE_RECTANGLE, 0, GL_RGBA8UI, totalScreenWidth, totalScreenHeight, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, &imageData[0] );
  glBindImageTexture( 0, renderTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8UI );


  // next, the initial heightmap data
  imageData.clear();
  imageData.resize( totalScreenWidth * totalScreenHeight * 4 );


  // finally, the initial colormap data
  imageData.clear();
  imageData.resize( totalScreenWidth * totalScreenHeight * 4 );




  cout << "done." << endl;

  // compute shader compilation
  cout << "  Compiling Compute Shaders..........................";
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
  glUniform2i( glGetUniformLocation( renderShader, "resolution" ), screenX, screenY );
  glDispatchCompute( std::ceil( totalScreenWidth / 64. ), 1, 1 );



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

  // put imgui data into the framebuffer
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

  // swap the double buffers
  SDL_GL_SwapWindow( window );
}

void engine::handleInput(){
  SDL_Event event;
  ImGuiIO &io = ImGui::GetIO();

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
