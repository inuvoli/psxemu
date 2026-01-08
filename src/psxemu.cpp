#include <loguru.hpp>

#include "psxemu.h"

//-------------------------------------------------------------------------------------------------------------
//
// OpenGL Error Message Callback
//
//-------------------------------------------------------------------------------------------------------------
void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
    LOG_F(WARNING,  "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s",
                    ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
                    type, severity, message );
}

psxemu::psxemu()
{
	pGamepadA = nullptr;
	pGamepadB = nullptr; 
    pWindow = nullptr;
}

psxemu::~psxemu()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(pWindow);
    SDL_Quit();
}

bool psxemu::init(int wndWidth, int wndHeight)
{
    //Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        LOG_F(ERROR, "SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return false;
    }

    LOG_F(INFO, "SDL Initialized...");

    //Create Gamepads
    int GamepadsIDs;
    SDL_GetGamepads(&GamepadsIDs);
    LOG_F(INFO, "Scanning Gamepads....detected [%d]", GamepadsIDs);


    for (int i = 0; i < GamepadsIDs; i++)
    {
        if (SDL_IsGamepad(i))
        {
            LOG_F(INFO, "Configuring Gamepad [%d]", i + 1);
            pGamepadA = SDL_OpenGamepad(i);
        }

    }

    // set OpenGL Version
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, opengl_major_version);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, opengl_minor_version);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    pWindow = SDL_CreateWindow("PSXemu", wndWidth, wndHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (pWindow == nullptr)
    {
        LOG_F(ERROR, "Window could not be created! SDL_Error: %s", SDL_GetError());
        return false;
    }
    LOG_F(INFO, "PSXEMU Window Created...");
     
    //Set window minimum size
    SDL_SetWindowMinimumSize(pWindow, wndWidth, wndHeight);

    //Set OpenGL Context
    glContext = SDL_GL_CreateContext(pWindow);
    SDL_GL_MakeCurrent(pWindow, glContext);
    SDL_GL_SetSwapInterval(0); // Enable vSync
    
    //Init OpenGL Loader
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        LOG_F(ERROR, "Failed to initialize OpenGL Loader...Error: %s", glewGetErrorString(err));
        return false;
    }
    LOG_F(INFO, "OpenGL Loader Initialized...");

    //Set VievPort
    glViewport(0, 0, wndWidth, wndHeight);
    
    //Set Default Clear Color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //Disable Blending
    glDisable(GL_BLEND);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
           
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(pWindow, glContext);
    ImGui_ImplOpenGL3_Init(glsl_version);
        
    //Init OpenGL Debug Callback
    //glEnable(GL_DEBUG_OUTPUT);
    //glDebugMessageCallback(MessageCallback, 0);

    //Init PSX Emulator Object
    isRunning = true;
    pPsx = std::make_shared<Psx>();

    return true;
}

bool psxemu::run()
{
    while (isRunning)
    {
        uint64_t timerStart = SDL_GetPerformanceCounter();

        //Handle SDL Events
        handleEvents();

        //Update Emulator Status
#ifdef DEBUGGER_ENABLED
        update(debugger::instance().getStepMode());
#else
        update();
#endif

        //Render Screen
        render();

        uint64_t timerActual = SDL_GetPerformanceCounter();
        uint16_t framePerSecond = static_cast<uint16_t>(SDL_GetPerformanceFrequency() / (timerActual - timerStart));
        debugger::instance().setFrameRate(framePerSecond);
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
//
// Private Methods
//
//----------------------------------------------------------------------------------------------------------------------
bool psxemu::handleEvents()
{
    int wndWidth;
    int wndHeight;

    //Handle events on queue
    while (SDL_PollEvent(&sdlEvent))
    {
        //Forward Events to ImGui
        ImGui_ImplSDL3_ProcessEvent(&sdlEvent);

        switch (sdlEvent.type)
        {
        case SDL_EVENT_QUIT:  //User requests quit
            isRunning = false;
            break;

        case SDL_EVENT_WINDOW_RESIZED:
            wndWidth = sdlEvent.window.data1;
            wndHeight = sdlEvent.window.data2;
            wndWidth = wndHeight * 4 / 3;
            SDL_SetWindowSize(pWindow, wndWidth, wndHeight);
            glViewport(0, 0, wndWidth, wndHeight);
            break;
    
        case SDL_EVENT_KEY_DOWN:          
            switch (sdlEvent.key.key)
            {
            case SDLK_X:
                isRunning = false;
                break;
            case SDLK_R:
                pPsx->reset();
                break;
            case SDLK_SPACE:
                debugger::instance().setStepMode(StepMode::Manual);
                break;
            case SDLK_P:
                debugger::instance().setStepMode(StepMode::Halt);
                break;
            case SDLK_I:
                debugger::instance().setStepMode(StepMode::Instruction);
                break;
            case SDLK_Y:
                debugger::instance().setStepMode(StepMode::Frame);
                break;
            case SDLK_M:
                pPsx->cpu->cop0->reg[12] = 0x00000401;
                break;
            case SDLK_1:
                debugger::instance().toggleDebugModuleStatus(DebugModule::Bios);
                break;
            case SDLK_2:
                debugger::instance().toggleDebugModuleStatus(DebugModule::Ram);
                break;
            case SDLK_3:
                debugger::instance().toggleDebugModuleStatus(DebugModule::Cpu);
                break;
            case SDLK_4:
                debugger::instance().toggleDebugModuleStatus(DebugModule::Code);
                break;
            case SDLK_5:
                debugger::instance().toggleDebugModuleStatus(DebugModule::Dma);
                break;
            case SDLK_6:
                debugger::instance().toggleDebugModuleStatus(DebugModule::Timers);
                break;
            case SDLK_7:
                debugger::instance().toggleDebugModuleStatus(DebugModule::Gpu);
                break;
            case SDLK_8:
                debugger::instance().toggleDebugModuleStatus(DebugModule::Spu);
                break;
            case SDLK_9:
                debugger::instance().toggleDebugModuleStatus(DebugModule::Cdrom);
                break;
            case SDLK_0:
                debugger::instance().toggleDebugModuleStatus(DebugModule::Tty);
                break;
            }    
            break;
        }
    }

    return true;
}

bool psxemu::update()
{
    //Run PSX Emulator in Normal mode
    while (!pPsx->gpu->isFrameReady())
        pPsx->execute();

    return true;
}

bool psxemu::update(StepMode stepMode)
{
    //Run PSX Emulator in Debug mode  
    switch(stepMode)
    {
        case StepMode::Halt:
            break;

        case StepMode::Manual:
            pPsx->execute();
            debugger::instance().update();
            debugger::instance().setStepMode(StepMode::Halt);
            break;

        case StepMode::Instruction:
            pPsx->execute();
            debugger::instance().update();
            if (debugger::instance().isBreakpoint()) debugger::instance().setStepMode(StepMode::Halt);
            break;

        case StepMode::Frame:
            while (!pPsx->gpu->isFrameReady())
            {
                pPsx->execute();
                debugger::instance().update();
                if (debugger::instance().isBreakpoint())
                {
                    debugger::instance().setStepMode(StepMode::Halt);
                    break;
                }
            }
            break;
    }
    return true;
}

bool psxemu::render()
{
    //Clear Frame Buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //Render Current PSX Frame
    pPsx->gpu->pRenderer->RenderDrawData(); 
 
#ifdef DEBUGGER_ENABLED
    //Render ImGUI Debug Widgets
    debugger::instance().render();
#endif
 
    //Swap OpenGL FrameBuffer
    SDL_GL_SwapWindow(pWindow);
    
    return true;
}
