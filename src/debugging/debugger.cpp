#include <loguru.hpp>

#include "debugger.h"
#include "psx.h"
#include "functions_name.h"
#include "registers_name.h"

bool debugger::isBreakpoint()
{
    return ((psx->cpu->pc) == breakPoint);
}

void debugger::toggleDebugModuleStatus(DebugModule module)
{
    int index = static_cast<int>(module);

    //Toggle Module Debug Status
    debugModuleStatus[index] = (debugModuleStatus[index]) ? false : true;
};

bool debugger::getDebugModuleStatus(DebugModule module)
{
    int index = static_cast<int>(module);

    return debugModuleStatus[index];
};

bool debugger::update()
{
    getInterruptDebugInfo();
    getTimerDebugInfo();
    getGpuDebugInfo();
    getCdromDebugInfo();
    updateCallStack();

    return true;
};

bool debugger::render()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();

    ImGui::NewFrame();

    renderMenuBar();
    renderFpsWidget();

    if (getDebugModuleStatus(DebugModule::Bios))     renderBiosWidget();
    if (getDebugModuleStatus(DebugModule::Ram))      renderRamWidget();
    if (getDebugModuleStatus(DebugModule::Cpu))      renderCpuWidget();
    if (getDebugModuleStatus(DebugModule::Code))     renderCodeWidget();
    if (getDebugModuleStatus(DebugModule::Dma))      renderDmaWidget();
    if (getDebugModuleStatus(DebugModule::Timers))   renderTimersWidget();
    if (getDebugModuleStatus(DebugModule::Gpu))      renderGpuWidget();
    if (getDebugModuleStatus(DebugModule::Spu))      renderSpuWidget();
    if (getDebugModuleStatus(DebugModule::Cdrom))    renderCdromWidget();
    if (getDebugModuleStatus(DebugModule::Tty))      renderTtyWidget();
    
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }

    return true;
}

bool debugger::renderFpsWidget()
{
    ImGui::SetNextWindowSize(ImVec2(100, 30));
    ImVec2 widgetPosition = ImGui::GetMainViewport()->WorkPos;
    widgetPosition.x += ImGui::GetMainViewport()->WorkSize.x;
    widgetPosition.y += ImGui::GetMainViewport()->WorkSize.y;
    ImGui::SetNextWindowPos(widgetPosition, 0, ImVec2(1.0f, 1.0f));
    
    ImGui::Begin("FPS", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
    ImGui::Text("FPS: %.4d\t", framePerSecond);
    ImGui::End();

    return true;
}

bool debugger::renderBiosWidget()
{
    dbgRom.DrawWindow("BIOS", psx->bios->rom, BIOS_SIZE);
    return true;
}

bool debugger::renderRamWidget()
{
    dbgRam.DrawWindow("RAM", psx->mem->ram, RAM_SIZE);
    return true;
}

bool debugger::renderCpuWidget()
{
    ImGui::Begin("CPU Registers");
        ImGui::BeginTabBar("#tabs");
            if (ImGui::BeginTabItem("Raw"))
            {
                for (int i = 0; i < 32; i++)
                {
                    ImGui::Text("%2d", i);
                    ImGui::SameLine();
                    ImGui::Text("%s = 0x%08x", cpuRegisterName[i].c_str() , psx->cpu->gpr[i]);
                    
                    ImGui::SameLine();
                    if (cop0RegisterName[i] == "")
                        ImGui::TextColored(darkgrey_color, "%8s = 0x%08x", cop0RegisterName[i].c_str(), psx->cpu->cop0->reg[i]);
                    else
                        ImGui::Text("%8s = 0x%08x", cop0RegisterName[i].c_str(), psx->cpu->cop0->reg[i]);
                    
                    ImGui::SameLine();
                    if (cop2RegisterName[i] == "")
                        ImGui::TextColored(darkgrey_color, "%8s = 0x%08x", cop2RegisterName[i].c_str(), psx->cpu->cop2->reg.data[i]);
                    else
                        ImGui::Text("%8s = 0x%08x", cop2RegisterName[i].c_str(), psx->cpu->cop2->reg.data[i]);
                    
                    ImGui::SameLine();
                    if (cop2RegisterName[i+32] == "")
                        ImGui::TextColored(darkgrey_color, "%8s = 0x%08x", cop2RegisterName[i+32].c_str(), psx->cpu->cop2->reg.ctrl[i]);
                    else
                        ImGui::Text("%8s = 0x%08x", cop2RegisterName[i+32].c_str(), psx->cpu->cop2->reg.ctrl[i]);
                    
                    switch (i)
                    {
                    case 0:
                        ImGui::SameLine();
                        ImGui::TextColored(green_color, "pc = 0x%08x", psx->cpu->pc);
                        break;
                    case 1:
                        ImGui::SameLine();
                        ImGui::Text("hi = 0x%08x", psx->cpu->hi);
                        break;
                    case 2:
                        ImGui::SameLine();
                        ImGui::Text("lo = 0x%08x", psx->cpu->lo);
                        break;
                    case 4:
                        ImGui::SameLine();
                        ImGui::Text("Cache Reg   = 0x%08x", psx->cpu->cacheReg);
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
                        ImGui::Text("POST = %d", psx->postStatus);
                        break;
                    }
                }
            ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("Call Stack"))
            {
                if (ImGui::BeginTable("Call Stack", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV))
                {
                    ImGui::TableSetupColumn("Jump Address", ImGuiTableColumnFlags_WidthFixed, 110.0f);
                    ImGui::TableSetupColumn("Program Counter", ImGuiTableColumnFlags_WidthFixed, 110.0f);
                    ImGui::TableSetupColumn("Stack Pointer", ImGuiTableColumnFlags_WidthFixed, 110.0f);
                    ImGui::TableSetupColumn("Return Address", ImGuiTableColumnFlags_WidthFixed, 110.0f);
                    ImGui::TableSetupColumn("Function Name");
                    
                    ImGui::PushStyleColor(ImGuiCol_Text, green_color);
                    ImGui::TableHeadersRow();
                    ImGui::PopStyleColor();       
                    
                    for (int i = 0; i < callStack.lenght() - 1; i++)
                    {   
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        CallStackInfo calledFunc = callStack.inspect(i);
                        ImGui::Text("0x%08x", calledFunc.jumpaddr); ImGui::TableNextColumn();
                        ImGui::Text("0x%08x", calledFunc.pc); ImGui::TableNextColumn();
                        ImGui::Text("0x%08x", calledFunc.sp); ImGui::TableNextColumn();
                        if (calledFunc.ra == 0x0)
                        {
                            ImGui::TextColored(darkgrey_color, "0x%08x", calledFunc.ra);
                            ImGui::TableNextColumn();
                        }
                        else
                        {
                            ImGui::Text("0x%08x", calledFunc.ra); ImGui::TableNextColumn();
                        }
                        ImGui::TextColored(grey_color, "%s", calledFunc.func.c_str());
                    }
                }
                ImGui::EndTable();
            ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("COP0"))
            {
                //TODO
            ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("COP2"))
            {
                //TODO
            ImGui::EndTabItem();
            }    
        ImGui::EndTabBar();
    ImGui::End();

    return true;
}

bool debugger::renderCodeWidget()
{
    ImGui::Begin("Assembler");
    uint32_t addr = psx->cpu->pc;

    //Debug on the fly software in RAM
    //Check if code is already disassembled, if not disassemble starting from current Program Counter.
    if (asmCode.find(addr) == asmCode.end() || std::get<2>(asmCode[addr]) == "")
    {
        AsmCode newDisassembledCode = mipsDisassembler.disassemble(psx->bios->rom, psx->mem->ram, addr);
        for (auto& e : newDisassembledCode)
            asmCode.insert_or_assign(e.first, e.second);
    }
    
    addr -= (15 * 4);
    for (int line = 0; line < 32; line++)
    {
        if (addr == psx->cpu->pc)
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
}

bool debugger::renderDmaWidget()
{
    ImGui::Begin("DMA");
            
    for (int i = 0; i < DMA_CHANNEL_NUMBER; i++)
    {
        ImGui::Text("DMA%d:MADR 0x%08x  ", i, psx->dma->dmaChannel[i].chanMadr);
        ImGui::SameLine();
        ImGui::Text("DMA%d:BCR  0x%08x  ", i, psx->dma->dmaChannel[i].chanBcr.word);
        ImGui::SameLine();
        ImGui::Text("DMA%d:CHCR 0x%08x  ", i, psx->dma->dmaChannel[i].chanChcr.word);
    }
    ImGui::TextColored(green_color, "DPCR 0x%08x       ", psx->dma->dmaDpcr);
    ImGui::SameLine();
    ImGui::TextColored(green_color, "DICR 0x%08x  ", psx->dma->dmaDicr.word);

    ImGui::End();

    return true;
}

bool debugger::renderTimersWidget()
{
    ImGui::Begin("TIMERS");
    for (int i = 0; i < TIMER_NUMBER; i++)
    {
        ImGui::Text("Timer %d: Value  0x%08x  ", i, timerInfo.timerStatus[i].counterValue);
        ImGui::SameLine();
        ImGui::Text("Timer %d: Mode   0x%08x  ", i, timerInfo.timerStatus[i].counterMode.word);
        ImGui::SameLine();
        ImGui::Text("Timer %d: Target 0x%08x  ", i, timerInfo.timerStatus[i].counterTarget);
    }
    ImGui::End();

    return true;
}

bool debugger::renderGpuWidget()
{
    ImGui::Begin("GPU");
    ImGui::Text("GPUSTAT: Value  0x%08x  ", gpuInfo.gpuStat);
    ImGui::Text("VideoMode     : %dx%d   (%s)", gpuInfo.videoResolution.x, gpuInfo.videoResolution.y, gpuInfo.videoStandard.c_str());

    ImGui::Text("Display Start : (%4d, %4d)\t\t\t\t\t\t", gpuInfo.displayStart.x, gpuInfo.displayStart.y);
    ImGui::SameLine();
    ImGui::Text("Texture Page           : (%4d, %4d)", gpuInfo.texturePage.x, gpuInfo.texturePage.y);

    ImGui::Text("Display Range : (%4d, %4d) to (%4d, %4d)\t\t", gpuInfo.displayRange.x1, gpuInfo.displayRange.y1, gpuInfo.displayRange.x2, gpuInfo.displayRange.y2);
    ImGui::SameLine();
    ImGui::Text("Texture Page Color Mode: %s", gpuInfo.textureColorDepth.c_str());

    ImGui::Text("Drawing Offset: (%4d, %4d)\t\t\t\t\t\t", (int16_t)gpuInfo.drawingOffset.x, (int16_t)gpuInfo.drawingOffset.y);
    ImGui::SameLine();
    ImGui::Text("Texture Windows Mask   : (%4d, %4d)", gpuInfo.textureMask.x, gpuInfo.textureMask.y);

    ImGui::Text("Drawing Area  : (%4d, %4d) to (%4d, %4d)\t\t", gpuInfo.drawingArea.x1, gpuInfo.drawingArea.y1, gpuInfo.drawingArea.x2, gpuInfo.drawingArea.y2);
    ImGui::SameLine();
    ImGui::Text("Texture Windows Offset : (%4d, %4d)", gpuInfo.textureOffset.x, gpuInfo.textureOffset.y); 
    
    glBindTexture(GL_TEXTURE_2D, vramTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, (const void*)gpuInfo.vRam);
            
    ImGui::Image((void*)(intptr_t)vramTexture, ImVec2(1024, 512));
    ImGui::End();

    return true;      
}

bool debugger::renderSpuWidget()
{
    return false;
}

bool debugger::renderCdromWidget()
{
    ImGui::Begin("CDROM");
    ImGui::Text("Status Register:           Value  0x%02x  ", cdromInfo.statusRegister);
    ImGui::Text("Request Register:          Value  0x%02x  ", cdromInfo.requestRegister);
    ImGui::Text("Interrupt Enable Register: Value  0x%02x  ", cdromInfo.interruptEnableRegister);
    ImGui::Text("Interrupt Flag Register:   Value  0x%02x  ", cdromInfo.interruptFlagRegister);
    ImGui::End();
    return false;
}

bool debugger::renderTtyWidget()
{
    ImGui::Begin("TTY");
    for (std::string s : psx->tty->bufferA)
        ImGui::Text("%s", s.c_str());
    ImGui::SetScrollHereY(1.0f);
    ImGui::End();
    return true;
}

bool debugger::renderMenuBar()
{
    // Menu Bar
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open Bios..."))
            {

            }
            if (ImGui::MenuItem("Open Game..."))
            {

            }
            if (ImGui::MenuItem("Exit", "X"))
            {

            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("ROM (Bios)", "1"))
            {
                toggleDebugModuleStatus(DebugModule::Bios);
            }
            if (ImGui::MenuItem("RAM", "2"))
            {
                toggleDebugModuleStatus(DebugModule::Ram);
            }
            if (ImGui::MenuItem("CPU Registers", "3"))
            {
                toggleDebugModuleStatus(DebugModule::Cpu);
            }
            if (ImGui::MenuItem("Disassembler", "4"))
            {
                toggleDebugModuleStatus(DebugModule::Code);
            }
            if (ImGui::MenuItem("DMA", "5"))
            {
                toggleDebugModuleStatus(DebugModule::Dma);
            }
            if (ImGui::MenuItem("Timers", "6"))
            {
                toggleDebugModuleStatus(DebugModule::Timers);
            }
            if (ImGui::MenuItem("GPU State", "7"))
            {
                toggleDebugModuleStatus(DebugModule::Gpu);
            }
            if (ImGui::MenuItem("SPU State", "8"))
            {
                toggleDebugModuleStatus(DebugModule::Spu);
            }
            if (ImGui::MenuItem("CDROM State", "9"))
            {
                toggleDebugModuleStatus(DebugModule::Cdrom);
            }
            if (ImGui::MenuItem("TTY", "0"))
            {
                toggleDebugModuleStatus(DebugModule::Tty);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug"))
        {
            if (ImGui::MenuItem("Dump RAM..."))
            {
                dumpRam();
            }
            if (ImGui::MenuItem("Set Breakpoint"))
            {

            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    return true;
}

void debugger::dumpRam()
{
    std::ofstream fs("ramdump.bin", std::ios::out | std::ios::binary);
    fs.write(reinterpret_cast<const char*>(psx->mem->ram), 0x200000);
    fs.close();
}

//------------------------------------------------------------------------------------------
// Debug Info Update Functions
//------------------------------------------------------------------------------------------
void debugger::getInterruptDebugInfo()
{
    interruptInfo.i_mask = psx->interrupt->getMaskRegister();
    interruptInfo.i_stat = psx->interrupt->getStatusRegister();
}

void debugger::getTimerDebugInfo()
{
    timerInfo.timerStatus[0] = psx->timers->getTimerStatus(0);
    timerInfo.timerStatus[1] = psx->timers->getTimerStatus(1);
    timerInfo.timerStatus[2] = psx->timers->getTimerStatus(2);
}

void debugger::getGpuDebugInfo()
{
    gpuInfo.gpuStat =  psx->gpu->getGPUStat();
    gpuInfo.videoResolution = psx->gpu->getVideoResolution();
    gpuInfo.displayRange = psx->gpu->getDisplayRange();
    gpuInfo.displayStart = psx->gpu->getDisplayStart();
    gpuInfo.drawingArea = psx->gpu->getDrawingArea();
    gpuInfo.drawingOffset = psx->gpu->getDrawingOffset();
    gpuInfo.textureDisabled = psx->gpu->getTextureDisabled();
    gpuInfo.texturePage = psx->gpu->getTexturePage();
    gpuInfo.textureMask = psx->gpu->getTextureMask();
    gpuInfo.textureOffset = psx->gpu->getTextureOffset();
    gpuInfo.vRam = psx->gpu->getVRAM();
    gpuInfo.textureColorDepth = psx->gpu->getTextureColorDepth();
    gpuInfo.videoStandard = psx->gpu->getVideoStandard();
}

void debugger::getCdromDebugInfo()
{
    cdromInfo.statusRegister = psx->cdrom->getStatusRegister();
    cdromInfo.requestRegister = psx->cdrom->getRequestRegister();
    cdromInfo.interruptEnableRegister = psx->cdrom->getInterruptEnableRegister();
    cdromInfo.interruptFlagRegister = psx->cdrom->getInterruptFlagRegister();   
}

void debugger::updateCallStack()
{
    //Check if the current instruction is a jump instruction
    if (mipsDisassembler.isJumpInstruction(psx->bios->rom, psx->mem->ram, psx->cpu->pc))
    {
        CallStackInfo callInfo;
        //callInfo.jumpaddr = psx->cpu->branchAddress;
        callInfo.pc = psx->cpu->pc;
        callInfo.sp = psx->cpu->gpr[29]; //Stack Pointer
        callInfo.ra = psx->cpu->gpr[31]; //Return Address TODO: should be 0x0 for J and JR instructions
        
        //callInfo.func = mipsDisassembler.getFunctionName(callInfo.jumpaddr);

        //std::stringstream ss;
	    //ss << "function_" << std::hex << callInfo.jumpaddr;
	    //callInfo.func = ss.str();

        //DEBUG - stdlib calls
        //A Functions
        if ((psx->cpu->pc & 0x1fffffff) == 0xac)
        {
            //LOG_F(1, "CPU - %s (r4: 0x%08x, r5: 0x%08x, r6: 0x%08x, r7: 0x%08x) [A(%02xh)]", function_A[gpr[9]].c_str(), gpr[4], gpr[5], gpr[6],gpr[7], gpr[9]);
            //tmp.func = function_A[gpr[9]];
        }
        //B Functions
        if ((psx->cpu->pc & 0x1fffffff) == 0xbc)
        {
            //LOG_F(1, "CPU - %s (r4: 0x%08x, r5: 0x%08x, r6: 0x%08x, r7: 0x%08x) [B(%02xh)]", function_B[gpr[9]].c_str(), gpr[4], gpr[5], gpr[6],gpr[7], gpr[9]);
            //tmp.func = function_B[gpr[9]];
        }
        //C Functions
        if ((psx->cpu->pc & 0x1fffffff) == 0xcc)
        {
            //LOG_F(1, "CPU - %s (r4: 0x%08x, r5: 0x%08x, r6: 0x%08x, r7: 0x%08x) [C(%02xh)]", function_C[gpr[9]].c_str(), gpr[4], gpr[5], gpr[6],gpr[7], gpr[9]);
            //tmp.func = function_C[gpr[9]];
        }

        callStack.write(callInfo);
    }
}