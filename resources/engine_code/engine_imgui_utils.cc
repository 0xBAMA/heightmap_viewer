#include "engine.h"

void engine::quitConf(bool *open) {
  if (*open) {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration;

    // create centered window
    ImGui::SetNextWindowPos(ImVec2(totalScreenWidth / 2 - 120, totalScreenHeight / 2 - 25));
    ImGui::SetNextWindowSize(ImVec2(300, 55));
    ImGui::Begin("quit", open, flags);

    ImGui::Text("Are you sure you want to quit?");

    ImGui::Text("  ");
    ImGui::SameLine();

    // button to cancel -> set this window's bool to false
    if (ImGui::Button(" Cancel "))
      *open = false;

    ImGui::SameLine();
    ImGui::Text("      ");
    ImGui::SameLine();

    // button to quit -> set programQuitFlag to true
    if (ImGui::Button(" Quit "))
      programQuitFlag = true;

    ImGui::End();
  }
}


void engine::ImGUIConfigure(){
  cout << "  Configuring ImGUI..................................";

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  // enable docking
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  // Setup Platform/Renderer bindings
  const char *glsl_version = "#version 430"; // OpenGL 4.3 + GLSL version 430
  ImGui_ImplSDL2_InitForOpenGL(window, GLcontext);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Theme shit
  ImVec4 *colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_Text] = ImVec4(0.67f, 0.50f, 0.16f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.33f, 0.27f, 0.16f, 1.00f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.05f, 0.00f, 1.00f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.23f, 0.17f, 0.02f, 0.05f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.30f, 0.12f, 0.06f, 0.94f);
  colors[ImGuiCol_Border] = ImVec4(0.25f, 0.18f, 0.09f, 0.33f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.33f, 0.15f, 0.02f, 0.17f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.561f, 0.082f, 0.04f, 0.17f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.09f, 0.02f, 0.17f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.25f, 0.12f, 0.01f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.25f, 0.12f, 0.01f, 1.00f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.07f, 0.02f, 1.00f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.10f, 0.08f, 0.53f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_CheckMark] = ImVec4(0.69f, 0.45f, 0.11f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.18f, 0.06f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.36f, 0.22f, 0.06f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_Header] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.18f, 0.06f, 0.37f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.33f, 0.15f, 0.02f, 0.17f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.42f, 0.18f, 0.06f, 0.17f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_Tab] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
  colors[ImGuiCol_TabActive] = ImVec4(0.34f, 0.14f, 0.01f, 1.00f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.42f, 0.18f, 0.06f, 1.00f);
  colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.06f, 0.03f, 0.01f, 0.78f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(0.64f, 0.42f, 0.09f, 0.90f);
  colors[ImGuiCol_NavHighlight] = ImVec4(0.64f, 0.42f, 0.09f, 0.90f);
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

  ImGuiStyle &style = ImGui::GetStyle();

  style.TabRounding = 2;
  style.FrameRounding = 2;
  style.WindowPadding.x = 0;
  style.WindowPadding.y = 0;
  style.FramePadding.x = 1;
  style.FramePadding.y = 0;
  style.IndentSpacing = 8;
  style.WindowRounding = 3;
  style.ScrollbarSize = 10;

  cout << "done." << endl;
}



// for the text editor
void engine::textEditor(){
  ImGui::Begin("Editor", NULL, 0);

  static TextEditor editor;
  static auto lang = TextEditor::LanguageDefinition::GLSL();
  editor.SetLanguageDefinition(lang);

  auto cpos = editor.GetCursorPosition();
  editor.SetPalette(TextEditor::GetDarkPalette());

  static const char *fileToEdit = "resources/engine_code/shaders/blit.vs.glsl";
  std::ifstream t(fileToEdit);
  static bool loaded = false;
  if (!loaded) {
    editor.SetLanguageDefinition(lang);
    if (t.good()) {
      editor.SetText(std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>()));
      loaded = true;
    }
  }

  ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1,
              cpos.mColumn + 1, editor.GetTotalLines(),
              editor.IsOverwrite() ? "Ovr" : "Ins",
              editor.CanUndo() ? "*" : " ",
              editor.GetLanguageDefinition().mName.c_str(), fileToEdit);

  editor.Render("TextEditor");
  ImGui::End();
}


void engine::adjustmentWindow(){
  ImGui::Begin( "Renderer State", NULL, 0);

  ImGui::Text( " Adjustment of Render parameters");

  ImGui::Indent();
  ImGui::SliderInt( "Height", &viewerHeight, 0, 500, "%d" );
  ImGui::SliderFloat2( "Position", (float*)&viewPosition, 0, 1024, "%.3f" );
  ImGui::SliderFloat( "Angle", &viewAngle, -3.14159265, 3.14159265, "%.3f" );
  ImGui::SliderFloat( "Max Distance", &maxDistance, 10, 5000, "%.3f" );
  ImGui::SliderInt( "Horizon", &horizonLine, 0, 3000, "%d" );
  ImGui::SliderFloat( "Height Scale", &heightScalar, 0, 900., "%.3f" );
  ImGui::SliderFloat( "Side-to-Side Offset", &offsetScalar, 0, 300., "%.3f" );
  ImGui::SliderFloat( "Step Increment", &stepIncrement, 0., 0.5, "%.3f" );
  ImGui::SliderFloat( "FoV", &FoVScalar, 0.001, 15.0, "%.3f" );
  ImGui::Text( "" );
  ImGui::SliderFloat( "Fog Scale", &fogScalar, 0., 1.5, "%.3f" );
  ImGui::ColorEdit3( "Fog Color", ( float * )&clearColor, 0 );
  ImGui::Text( "" );

  const char* items[] = {
    "Map  1", "Map  2", "Map  3", "Map  4", "Map  5",
    "Map  6", "Map  7", "Map  8", "Map  9", "Map 10",
    "Map 11", "Map 12", "Map 13", "Map 14", "Map 15",
    "Map 16", "Map 17", "Map 18", "Map 19", "Map 20",
    "Map 21", "Map 22", "Map 23", "Map 24", "Map 25",
    "Map 26", "Map 27", "Map 28", "Map 29", "Map 30"};

  static int mapPickerItemCurrent  = 0;
  static int mapPickerItemPrevious = 0;
  ImGui::Combo("Map Picker", &mapPickerItemCurrent, items, IM_ARRAYSIZE(items));

  if( mapPickerItemCurrent != mapPickerItemPrevious ){
    mapPickerItemPrevious = mapPickerItemCurrent;
    loadMap( mapPickerItemCurrent + 1 );
  }
  ImGui::Unindent();
  ImGui::Text( std::string( " Frame Time: " + std::to_string( prevFrameTimeMs ) + " ms ( " + std::to_string( 1. / ( prevFrameTimeMs / 1000. ) ) + " fps )" ).c_str() );

  ImGui::End();
}
