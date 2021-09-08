#include "psxemu.h"

psxemu::psxemu()
{
    //Debug Status
    showRegister = false;
    showRom = false;
    showRam = false;
    showAssembler = false;
    showDma = false;
    showTimers = false;
    showGpu = false;
    breakPoint = 0xffffffff;

	isRunning = false;
    isHalt = true;
    isFast = false;

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

    //Delete PSX Emulator Object
    delete pPsxSystem;
}

bool psxemu::init(int wndWidth, int wndHeight)
{
    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    printf("SDL Initialized...\n");
        
    //Create Controller
    printf("Scanning Controllers....detected [%d]\n", SDL_NumJoysticks());

    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        if (SDL_IsGameController(i))
        {
            printf("Configuring Controller [%d]\n", i + 1);
            pControllerA = SDL_GameControllerOpen(i);
        }

    }

    // OpenGL 3.3 + GLSL v. 330
    const char* glsl_version = "#version 330 core";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    pWindow = SDL_CreateWindow("PSXEmu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, wndWidth, wndHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (pWindow == nullptr)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    printf("Window Created...\n");
     
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
        printf("Failed to initialize OpenGL Loader...Error: %s\n", glewGetErrorString(err));
        return false;
    }
    printf("OpenGL Loader Initialized...\n");

    //Set Clear Color
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
           
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(pWindow, glContext);
    ImGui_ImplOpenGL3_Init(glsl_version);
 
    isRunning = true;

    //Init PSX Emulator Object
    pPsxSystem = new Psx();

    return true;
}

bool psxemu::run()
{
    while (isRunning)
    {
        timerStart = SDL_GetPerformanceCounter();

        //Handle SDL Events
        handleEvents();

        //Run Single Line
        if (!isHalt)
            update();

        //Update Full Frame
        if (isFast)
            updateFrame();

        renderFrame();

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

            case SDLK_SPACE:
                pPsxSystem->clock();
                break;

            case SDLK_r:
                pPsxSystem->reset();
                break;

            case SDLK_h:
                (isHalt) ? isHalt = false : isHalt = true;
                break;

            case SDLK_f:
                (isFast) ? isFast = false : isFast = true;
                break;

            case SDLK_1:
                (showRom) ? showRom = false : showRom = true;
                break;
            case SDLK_2:
                (showRam) ? showRam = false : showRam = true;
                break;
            case SDLK_3:
                (showRegister) ? showRegister = false : showRegister = true;
                break;
            case SDLK_4:
                (showAssembler) ? showAssembler = false : showAssembler = true;
                break;
            case SDLK_5:
                (showDma) ? showDma = false : showDma = true;
                break;
            case SDLK_6:
                (showTimers) ? showTimers = false : showTimers = true;
                break;
            case SDLK_7:
                (showGpu) ? showGpu = false : showGpu = true;
                break;
            }
            break;
        }
    }

    return true;
}

bool psxemu::debugInfo()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(pWindow);
    
    ImGui::NewFrame();
    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGui::Begin("FPS", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2(80, 30));
    ImGui::Text("FPS: %3d", framePerSecond);
    ImGui::End();

    if (showRom)
    {
        dbgRom.DrawWindow("BIOS", pPsxSystem->bios.rom, BIOS_SIZE);
    }

    if (showRam)
    {
        dbgRam.DrawWindow("RAM", pPsxSystem->mem.ram, RAM_SIZE);
    }
    
    if (showRegister)
    {
        ImGui::Begin("CPU Registers");
        for (int i = 0; i < 32; i++)
        {
            ImGui::Text("%2d", i);
            ImGui::SameLine();
            ImGui::Text("%s = 0x%08x", cpuRegisterName[i].c_str() , pPsxSystem->cpu.gpr[i]);
            ImGui::SameLine();
            if (cop0RegisterName[i] == "")
                ImGui::TextColored(darkgrey_color, "%8s = 0x%08x", cop0RegisterName[i].c_str(), pPsxSystem->cpu.cop0_reg[i]);
            else
                ImGui::Text("%8s = 0x%08x", cop0RegisterName[i].c_str(), pPsxSystem->cpu.cop0_reg[i]);
            switch (i)
            {
            case 0:
                ImGui::SameLine();
                ImGui::TextColored(green_color, "pc = 0x%08x", pPsxSystem->cpu.pc);
                break;
            case 1:
                ImGui::SameLine();
                ImGui::Text("hi = 0x%08x", pPsxSystem->cpu.hi);
                break;
            case 2:
                ImGui::SameLine();
                ImGui::Text("lo = 0x%08x", pPsxSystem->cpu.lo);
                break;
            case 4:
                ImGui::SameLine();
                ImGui::Text("Cache Reg   = 0x%08x", pPsxSystem->cpu.cacheReg);
                break;
            case 5:
                ImGui::SameLine();
                ImGui::Text("Int. Status = 0x%08x", pPsxSystem->cpu.intStatus);
                break;
            case 6:
                ImGui::SameLine();
                ImGui::Text("Int. Mask   = 0x%08x", pPsxSystem->cpu.intMask);
                break;
            case 8:
                ImGui::SameLine();
                ImGui::Text("POST = %d", pPsxSystem->postStatus);
                break;
            }
        }
        ImGui::End();
    }

    if (showAssembler)
    {
        ImGui::Begin("Assembler");
        uint32_t addr = pPsxSystem->cpu.memwbReg.pc;

        //Debug on the fly software in RAM
        if (pPsxSystem->codeList.find(addr) == pPsxSystem->codeList.end() || pPsxSystem->codeList[addr] == "")
        {
            std::map<uint32_t, std::string> newListing = pPsxSystem->cpu.disassemble(addr, addr+0x40);
            for (auto& e : newListing)
                pPsxSystem->codeList.insert_or_assign(e.first, e.second);
        }

        addr -= 0x40;
        for (int line = 0; line < 32; line++)
        {
            if (addr == pPsxSystem->cpu.pc)
            {
                ImGui::TextColored(grey_color, "%s", pPsxSystem->codeList[addr].c_str());
            }
            else if (addr == pPsxSystem->cpu.idexReg.pc)
            {
                ImGui::TextColored(grey_color, "%s", pPsxSystem->codeList[addr].c_str());
            }
            else if (addr == pPsxSystem->cpu.memwbReg.pc)
            {
                ImGui::TextColored(red_color, "%s", pPsxSystem->codeList[addr].c_str());
            }
            else
            {
                ImGui::TextColored(grey_color, "%s", pPsxSystem->codeList[addr].c_str());
            }
            addr += 4;
        }
        ImGui::End();
    }

    if (showDma)
    {
        ImGui::Begin("DMA");
                
        for (int i = 0; i < DMA_CHANNEL_NUMBER; i++)
        {
            ImGui::Text("DMA%d:MADR 0x%08x  ", i, pPsxSystem->dma.dmaChannel[i].chanMadr);
            ImGui::SameLine();
            ImGui::Text("DMA%d:BCR  0x%08x  ", i, pPsxSystem->dma.dmaChannel[i].chanBcr.word);
            ImGui::SameLine();
            ImGui::Text("DMA%d:CHCR 0x%08x  ", i, pPsxSystem->dma.dmaChannel[i].chanChcr.word);
        }
        ImGui::TextColored(green_color, "DPCR 0x%08x       ", pPsxSystem->dma.dmaDpcr);
        ImGui::SameLine();
        ImGui::TextColored(green_color, "DICR 0x%08x  ", pPsxSystem->dma.dmaDicr);

        ImGui::End();
    }

    if (showTimers)
    {
//        ImGui::Begin("TIMERS");

 //       for (int i = 0; i < TIMER_NUMBER; i++)
 //       {
 //           ImGui::Text("TIMER%d:Value  0x%08x  ", i, pPsxSystem->timers.timerCounterValue[i]);
 //           ImGui::SameLine();
 //           ImGui::Text("TIMER%d:Mode   0x%08x  ", i, pPsxSystem->timers.timerCounterMode[i].word);
 //           ImGui::SameLine();
 //           ImGui::Text("TIMER%d:Target 0x%08x  ", i, pPsxSystem->timers.timerCounterTarget[i]);
 //       }

//        ImGui::End();
    }

    if (showGpu)
    {
        GpuDebugInfo gpuinfo;
        pPsxSystem->gpu.getDebugInfo(gpuinfo);

        ImGui::Begin("GPU");
        ImGui::Text("GPUSTAT: Value  0x%08x  ", gpuinfo.gpuStat);
        ImGui::Text("VideoMode     : %dx%d   (%s)", gpuinfo.videoResolution.x, gpuinfo.videoResolution.y, gpuinfo.videoStandard.c_str());
        ImGui::Text("Display Offset: (%4d, %4d)", gpuinfo.displayOffset.x, gpuinfo.displayOffset.y);
        ImGui::Text("Display Area  : (%4d, %4d) to (%4d, %4d)", gpuinfo.displayArea.x1, gpuinfo.displayArea.y1, gpuinfo.displayArea.x2, gpuinfo.displayArea.y2);
        ImGui::Text("Drawing Offset: (%4d, %4d)", (int16_t)gpuinfo.drawingOffset.x, (int16_t)gpuinfo.drawingOffset.y);
        ImGui::Text("Drawing Area  : (%4d, %4d) to (%4d, %4d)", gpuinfo.drawingArea.x1, gpuinfo.drawingArea.y1, gpuinfo.drawingArea.x2, gpuinfo.drawingArea.y2);
                
        GLuint vramTexture;
        glGenTextures(1, &vramTexture);
        glBindTexture(GL_TEXTURE_2D, vramTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 512, 0, GL_ABGR_EXT, GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT, gpuinfo.vRam);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, gpuinfo.vRam);
        ImGui::Image((void*)(intptr_t)vramTexture, ImVec2(1024, 512));
        ImGui::End();

        ImGui::End();
    }
    
    ImGui::Render();
    

    return true;
}

bool psxemu::update()
{
    if (pPsxSystem->cpu.pc - 0x4 == breakPoint)
    {
        isHalt = true;
        isFast = false;
    }

    pPsxSystem->clock();
    return true;
}

bool psxemu::updateFrame()
{
    int count = 0;
    
    while (!pPsxSystem->gpu.isFrameReady())
    {
        if (pPsxSystem->cpu.pc - 0x4 == breakPoint)
        {
            isHalt = true;
            isFast = false;
            break;
        }

        pPsxSystem->clock();
    }
        
    return true;
}

bool psxemu::renderFrame()
{
    //Update Debug Frame
    debugInfo();

    //Clear ColorBuffer
    glClear(GL_COLOR_BUFFER_BIT);

    //Render PSP FrameBuffer
    //TODO
    
    //Render Debug Information
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    //Swap OpenGL FrameBuffer
    SDL_GL_SwapWindow(pWindow);

    return true;
}
