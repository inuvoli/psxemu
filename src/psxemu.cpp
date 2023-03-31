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

    // pWindow2 = SDL_CreateWindow("PSX Debugger", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, wndWidth, wndHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    // if (pWindow == nullptr)
    // {
    //     LOG_F(ERROR, "Window could not be created! SDL_Error: %s", SDL_GetError());
    //     return false;
    // }
    //LOG_F(INFO, (INFO, "DEBUGGER Window Created...");
     
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
           
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(pWindow, glContext);
    ImGui_ImplOpenGL3_Init(glsl_version);
     
    //Init OpenGL Variables
    glGenTextures(1, &vramTexture);

    //Init OpenGL Debug Callback
    //glEnable(GL_DEBUG_OUTPUT);
    //glDebugMessageCallback(MessageCallback, 0);

    //Init PSX Emulator Object
    isRunning = true;
    pPsx = std::make_shared<Psx>(biosFileName, gameFileName);

    //Init PSX Debugger
    pDebugger = std::make_shared<Debugger>(pPsx);   

    return true;
}

bool psxemu::run()
{
    while (isRunning)
    {
        timerStart = SDL_GetPerformanceCounter();

        //Handle SDL Events
        handleEvents();

        //Update Emulator Status
        update(pDebugger->getStepMode());

        //Render Screen
        render(pDebugger->getStepMode()); 

        timerStop = SDL_GetPerformanceCounter();
        framePerSecond = static_cast<uint16_t>(SDL_GetPerformanceFrequency() / (timerStop - timerStart));
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
            case SDLK_q:
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
            case SDLK_F1:
                pDebugger->toggleDebugModuleStatus(DebugModule::Bios);
                break;
            case SDLK_F2:
                pDebugger->toggleDebugModuleStatus(DebugModule::Ram);
                break;
            case SDLK_F3:
                pDebugger->toggleDebugModuleStatus(DebugModule::Cpu);
                break;
            case SDLK_F4:
                pDebugger->toggleDebugModuleStatus(DebugModule::Code);
                break;
            case SDLK_F5:
                pDebugger->toggleDebugModuleStatus(DebugModule::Dma);
                break;
            case SDLK_F6:
                pDebugger->toggleDebugModuleStatus(DebugModule::Timers);
                break;
            case SDLK_F7:
                pDebugger->toggleDebugModuleStatus(DebugModule::Gpu);
                break;
            case SDLK_F8:
                pDebugger->toggleDebugModuleStatus(DebugModule::Cdrom);
                break;
            //-------------------------------- Test
            case SDLK_z:
                pDebugger->setBreakpoint(0x00000001);
                //pPsx->interrupt->set(static_cast<uint32_t>(interruptCause::cdrom));
                break;
            case SDLK_x:
                pDebugger->setBreakpoint(0x00000000);
                //pPsx->interrupt->set(static_cast<uint32_t>(interruptCause::cdrom));
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
            pPsx->clock();
            pDebugger->setStepMode(StepMode::Halt);
            break;

        case StepMode::Instruction:
            pPsx->clock();
            if (pDebugger->isBreakpoint()) pDebugger->setStepMode(StepMode::Halt);
            break;

        case StepMode::Frame:
            while (!pPsx->gpu->isFrameReady())
            {
                pPsx->clock();
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
    renderWidgets();

    //Swap OpenGL FrameBuffer
    SDL_GL_SwapWindow(pWindow);
    
    return true;
}

bool psxemu::renderWidgets()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(pWindow);
    
    ImGui::NewFrame();
    ImGui::SetNextWindowBgAlpha(0.35f);

    renderFpsWidget();
    if (pDebugger->getDebugModuleStatus(DebugModule::Bios))     renderBiosWidget();
    if (pDebugger->getDebugModuleStatus(DebugModule::Ram))      renderRamWidget();
    if (pDebugger->getDebugModuleStatus(DebugModule::Cpu))      renderCpuWidget();
    if (pDebugger->getDebugModuleStatus(DebugModule::Code))     renderCodeWidget();
    if (pDebugger->getDebugModuleStatus(DebugModule::Dma))      renderDmaWidget();
    if (pDebugger->getDebugModuleStatus(DebugModule::Timers))   renderTimersWidget();
    if (pDebugger->getDebugModuleStatus(DebugModule::Gpu))      renderGpuWidget();
    if (pDebugger->getDebugModuleStatus(DebugModule::Spu))      renderSpuWidget();
    if (pDebugger->getDebugModuleStatus(DebugModule::Cdrom))    renderCdromWidget();
    
    ImGui::Render();

    //Render Debug Information
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return true;
}

bool psxemu::renderFpsWidget()
{
    ImGui::Begin("FPS", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2(80, 30));
    ImGui::Text("FPS: %3d", framePerSecond);
    ImGui::End();

    return true;
};

bool psxemu::renderBiosWidget()
{
    dbgRom.DrawWindow("BIOS", pPsx->bios->rom, BIOS_SIZE);
    return true;
};

bool psxemu::renderRamWidget()
{
    dbgRam.DrawWindow("RAM", pPsx->mem->ram, RAM_SIZE);
    return true;
};

bool psxemu::renderCpuWidget()
{
    InterruptDebugInfo interruptInfo;
    pPsx->interrupt->getDebugInfo(interruptInfo);

    ImGui::Begin("CPU Registers");
    for (int i = 0; i < 32; i++)
    {
        ImGui::Text("%2d", i);
        ImGui::SameLine();
        ImGui::Text("%s = 0x%08x", cpuRegisterName[i].c_str() , pPsx->cpu->gpr[i]);
        ImGui::SameLine();
        if (cop0RegisterName[i] == "")
            ImGui::TextColored(darkgrey_color, "%8s = 0x%08x", cop0RegisterName[i].c_str(), pPsx->cpu->cop0->reg[i]);
        else
            ImGui::Text("%8s = 0x%08x", cop0RegisterName[i].c_str(), pPsx->cpu->cop0->reg[i]);
        switch (i)
        {
        case 0:
            ImGui::SameLine();
            ImGui::TextColored(green_color, "pc = 0x%08x", pPsx->cpu->pc);
            break;
        case 1:
            ImGui::SameLine();
            ImGui::Text("hi = 0x%08x", pPsx->cpu->hi);
            break;
        case 2:
            ImGui::SameLine();
            ImGui::Text("lo = 0x%08x", pPsx->cpu->lo);
            break;
        case 4:
            ImGui::SameLine();
            ImGui::Text("Cache Reg   = 0x%08x", pPsx->cpu->cacheReg);
            break;
        case 5:
            ImGui::SameLine();
            ImGui::Text("Int. Status = 0x%08x", interruptInfo.i_stat);
            break;
        case 6:
            ImGui::SameLine();
            ImGui::Text("Int. Mask   = 0x%08x", interruptInfo.i_mask);
            break;
        case 8:
            ImGui::SameLine();
            ImGui::Text("POST = %d", pPsx->postStatus);
            break;
        }
    }
    ImGui::End();

    return true;
};

bool psxemu::renderCodeWidget()
{
    ImGui::Begin("Assembler");
    uint32_t addr = pPsx->cpu->pc;

    //Debug on the fly software in RAM
    //Check if code is already disassembled, if not disassemble starting from current Program Counter.
    if (asmCode.find(addr) == asmCode.end() || std::get<2>(asmCode[addr]) == "")
    {
        AsmCode newDisassembledCode = mipsDisassembler.disassemble(pPsx->bios->rom, pPsx->mem->ram, addr);
        for (auto& e : newDisassembledCode)
            asmCode.insert_or_assign(e.first, e.second);
    }
    
    addr -= (15 * 4);
    for (int line = 0; line < 32; line++)
    {
        if (addr == pPsx->cpu->pc)
        {
            ImGui::TextColored(red_color, "0x%08x", addr);
            ImGui::SameLine();
            ImGui::TextColored(red_color, "0x%08x", std::get<0>(asmCode[addr]));
            ImGui::SameLine();
            ImGui::TextColored(red_color, "%s", std::get<1>(asmCode[addr]).c_str());
            ImGui::SameLine();
            ImGui::TextColored(red_color, "%s", std::get<2>(asmCode[addr]).c_str());
        }
        else
        {
            ImGui::TextColored(yellow_color, "0x%08x", addr);
            ImGui::SameLine();
            ImGui::TextColored(green_color, "0x%08x", std::get<0>(asmCode[addr]));
            ImGui::SameLine();
            ImGui::TextColored(grey_color, "%s", std::get<1>(asmCode[addr]).c_str());
            ImGui::SameLine();
            ImGui::TextColored(grey_color, "%s", std::get<2>(asmCode[addr]).c_str());
        }
        
        addr += 4;
    }
    ImGui::End();

    return true;
};

bool psxemu::renderDmaWidget()
{
    ImGui::Begin("DMA");
            
    for (int i = 0; i < DMA_CHANNEL_NUMBER; i++)
    {
        ImGui::Text("DMA%d:MADR 0x%08x  ", i, pPsx->dma->dmaChannel[i].chanMadr);
        ImGui::SameLine();
        ImGui::Text("DMA%d:BCR  0x%08x  ", i, pPsx->dma->dmaChannel[i].chanBcr.word);
        ImGui::SameLine();
        ImGui::Text("DMA%d:CHCR 0x%08x  ", i, pPsx->dma->dmaChannel[i].chanChcr.word);
    }
    ImGui::TextColored(green_color, "DPCR 0x%08x       ", pPsx->dma->dmaDpcr);
    ImGui::SameLine();
    ImGui::TextColored(green_color, "DICR 0x%08x  ", pPsx->dma->dmaDicr.word);

    ImGui::End();

    return true;
};

bool psxemu::renderTimersWidget()
{
    TimerDebugInfo timerinfo;
    pPsx->timers->getDebugInfo(timerinfo);

    ImGui::Begin("TIMERS");
    for (int i = 0; i < TIMER_NUMBER; i++)
    {
        ImGui::Text("Timer %d: Value  0x%08x  ", i, timerinfo.timerStatus[i].counterValue);
        ImGui::SameLine();
        ImGui::Text("Timer %d: Mode   0x%08x  ", i, timerinfo.timerStatus[i].counterMode.word);
        ImGui::SameLine();
        ImGui::Text("Timer %d: Target 0x%08x  ", i, timerinfo.timerStatus[i].counterTarget);
    }
    ImGui::End();

    return true;
};

bool psxemu::renderGpuWidget()
{
    GpuDebugInfo gpuinfo;
    pPsx->gpu->getDebugInfo(gpuinfo);

    ImGui::Begin("GPU");
    ImGui::Text("GPUSTAT: Value  0x%08x  ", gpuinfo.gpuStat);
    ImGui::Text("VideoMode     : %dx%d   (%s)", gpuinfo.videoResolution.x, gpuinfo.videoResolution.y, gpuinfo.videoStandard.c_str());
    
    ImGui::Text("Display Start : (%4d, %4d)\t\t\t\t\t\t", gpuinfo.displayStart.x, gpuinfo.displayStart.y);
    ImGui::SameLine();
    ImGui::Text("Texture Page           : (%4d, %4d)", gpuinfo.texturePage.x, gpuinfo.texturePage.y);

    ImGui::Text("Display Range : (%4d, %4d) to (%4d, %4d)\t\t", gpuinfo.displayRange.x1, gpuinfo.displayRange.y1, gpuinfo.displayRange.x2, gpuinfo.displayRange.y2);
    ImGui::SameLine();
    ImGui::Text("Texture Page Color Mode: %s", gpuinfo.texturePageColor.c_str());

    ImGui::Text("Drawing Offset: (%4d, %4d)\t\t\t\t\t\t", (int16_t)gpuinfo.drawingOffset.x, (int16_t)gpuinfo.drawingOffset.y);
    ImGui::SameLine();
    ImGui::Text("Texture Windows Mask   : (%4d, %4d)", gpuinfo.textureMask.x, gpuinfo.textureMask.y);

    ImGui::Text("Drawing Area  : (%4d, %4d) to (%4d, %4d)\t\t", gpuinfo.drawingArea.x1, gpuinfo.drawingArea.y1, gpuinfo.drawingArea.x2, gpuinfo.drawingArea.y2);
    ImGui::SameLine();
    ImGui::Text("Texture Windows Offset : (%4d, %4d)", gpuinfo.textureOffset.x, gpuinfo.textureOffset.y); 
    
    glBindTexture(GL_TEXTURE_2D, vramTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, (const void*)gpuinfo.vRam);
            
    ImGui::Image((void*)(intptr_t)vramTexture, ImVec2(1024, 512));
    ImGui::End();

    return true;      
};

bool psxemu::renderSpuWidget()
{
    return false;
};

bool psxemu::renderCdromWidget()
{
    CdromDebugInfo info;
    pPsx->cdrom->getDebugInfo(info);

    ImGui::Begin("CDROM");
    ImGui::Text("Status Register:           Value  0x%02x  ", info.statusRegister);
    ImGui::Text("Request Register:          Value  0x%02x  ", info.requestRegister);
    ImGui::Text("Interrupt Enable Register: Value  0x%02x  ", info.interruptEnableRegister);
    ImGui::Text("Interrupt Flag Register:   Value  0x%02x  ", info.interruptFlagRegister);
    ImGui::End();
    return false;
};