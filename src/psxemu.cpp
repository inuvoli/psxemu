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
	pControllerA = nullptr;
	pControllerB = nullptr; 
    pWindow = nullptr;
}

psxemu::~psxemu()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(pWindow);
    SDL_Quit();
}

bool psxemu::init(int wndWidth, int wndHeight, const std::string& biosFileName, const std::string& gameFileName)
{
    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) < 0)
    {
        LOG_F(ERROR, "SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return false;
    }
    
    LOG_F(INFO, "SDL Initialized...");
        
    //Create Controller
    LOG_F(INFO, "Scanning Controllers....detected [%d]", SDL_NumJoysticks());

    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        if (SDL_IsGameController(i))
        {
            LOG_F(INFO, "Configuring Controller [%d]", i + 1);
            pControllerA = SDL_GameControllerOpen(i);
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

    pWindow = SDL_CreateWindow("PSXEmu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, wndWidth, wndHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
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
    ImGui_ImplSDL2_InitForOpenGL(pWindow, glContext);
    ImGui_ImplOpenGL3_Init(glsl_version);
        
    //Init OpenGL Debug Callback
    //glEnable(GL_DEBUG_OUTPUT);
    //glDebugMessageCallback(MessageCallback, 0);

    //Init PSX Emulator Object
    isRunning = true;
    pPsx = std::make_shared<Psx>(biosFileName, gameFileName);

    //Init PSX Debugger
    pDebugger = std::make_shared<debugger>(pPsx);   

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
        update(pDebugger->getStepMode());

        //Render Screen
        render(pDebugger->getStepMode()); 

        uint64_t timerStop = SDL_GetPerformanceCounter();
        uint16_t framePerSecond = static_cast<uint16_t>(SDL_GetPerformanceFrequency() / (timerStop - timerStart));
        pDebugger->setFrameRate(framePerSecond);
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
    while (SDL_PollEvent(&sdlEvent) != 0)
    {
        //Forward Events to ImGui
        ImGui_ImplSDL2_ProcessEvent(&sdlEvent);

        switch (sdlEvent.type)
        {
        case SDL_QUIT:  //User requests quit
            isRunning = false;
            break;

        case SDL_WINDOWEVENT:
            switch (sdlEvent.window.event)
            {
            case SDL_WINDOWEVENT_RESIZED:
                wndWidth = sdlEvent.window.data1;
                wndHeight = sdlEvent.window.data2;
                wndWidth = wndHeight * 4 / 3;
                SDL_SetWindowSize(pWindow, wndWidth, wndHeight);
                glViewport(0, 0, wndWidth, wndHeight);
                break;
            }
            break;
        
        case SDL_KEYDOWN:
            switch (sdlEvent.key.keysym.sym)
            {
            case SDLK_x:
                isRunning = false;
                break;
            case SDLK_r:
                pPsx->reset();
                break;
            case SDLK_SPACE:
                pDebugger->setStepMode(StepMode::Manual);
                break;
            case SDLK_p:
                pDebugger->setStepMode(StepMode::Halt);
                break;
            case SDLK_i:
                pDebugger->setStepMode(StepMode::Instruction);
                break;
            case SDLK_y:
                pDebugger->setStepMode(StepMode::Frame);
                break;
            case SDLK_1:
                pDebugger->toggleDebugModuleStatus(DebugModule::Bios);
                break;
            case SDLK_2:
                pDebugger->toggleDebugModuleStatus(DebugModule::Ram);
                break;
            case SDLK_3:
                pDebugger->toggleDebugModuleStatus(DebugModule::Cpu);
                break;
            case SDLK_4:
                pDebugger->toggleDebugModuleStatus(DebugModule::Code);
                break;
            case SDLK_5:
                pDebugger->toggleDebugModuleStatus(DebugModule::Dma);
                break;
            case SDLK_6:
                pDebugger->toggleDebugModuleStatus(DebugModule::Timers);
                break;
            case SDLK_7:
                pDebugger->toggleDebugModuleStatus(DebugModule::Gpu);
                break;
            case SDLK_8:
                pDebugger->toggleDebugModuleStatus(DebugModule::Spu);
                break;
            case SDLK_9:
                pDebugger->toggleDebugModuleStatus(DebugModule::Cdrom);
                break;
            case SDLK_0:
                pDebugger->toggleDebugModuleStatus(DebugModule::Tty);
                break;
            }
            break;
        }
    }

    return true;
}

bool psxemu::update(StepMode stepMode)
{  
    switch(stepMode)
    {
        case StepMode::Halt:
            break;

        case StepMode::Manual:
            pPsx->execute();
            pDebugger->setStepMode(StepMode::Halt);
            break;

        case StepMode::Instruction:
            pPsx->execute();
            if (pDebugger->isBreakpoint()) pDebugger->setStepMode(StepMode::Halt);
            break;

        case StepMode::Frame:
            while (!pPsx->gpu->isFrameReady())
            {
                pPsx->execute();
                if (pDebugger->isBreakpoint())
                {
                    pDebugger->setStepMode(StepMode::Halt);
                    break;
                }
            }
            break;
    }

    //Update Debug Info
    pDebugger->update();
    
    return true;
}

bool psxemu::render(StepMode stepMode)
{
    //Clear Frame Buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //Render Current PSX Frame
    pPsx->gpu->pRenderer->RenderDrawData(); 
 
    //Render ImGUI Debug Widgets
    pDebugger->render();

    //Swap OpenGL FrameBuffer
    SDL_GL_SwapWindow(pWindow);
    
    return true;
}
