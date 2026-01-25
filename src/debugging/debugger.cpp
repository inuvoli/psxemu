#include <loguru.hpp>

#include <cstdlib>

#include "debugger.h"
#include "psx.h"
#include "functions_name.h"
#include "registers_name.h"

bool Debugger::isBreakpoint()
{
    return (instance().psx->cpu->pc == instance().breakPoint);
}

void Debugger::toggleDebugModuleStatus(DebugModule module)
{
    int index = static_cast<int>(module);

    //Toggle Module Debug Status
    instance().debugModuleStatus[index] = (instance().debugModuleStatus[index]) ? false : true;
};

bool Debugger::getDebugModuleStatus(DebugModule module)
{
    int index = static_cast<int>(module);

    return instance().debugModuleStatus[index];
};

bool Debugger::init()
{
    //Nothing to Do
    return false;
}

bool Debugger::update()
{
    //Nothing to Do
    //Debug Info updated inside each widget render function in order reduce unnecessary updates

    return false;
};

bool Debugger::render()
{
    auto& r = instance();  // Alias al singleton

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();

    ImGui::NewFrame();

    r.renderMenuBar();
    r.renderFpsWidget();

    if (r.getDebugModuleStatus(DebugModule::Bios))     r.renderBiosWidget();
    if (r.getDebugModuleStatus(DebugModule::Ram))      r.renderRamWidget();
    if (r.getDebugModuleStatus(DebugModule::Cpu))      r.renderCpuWidget();
    if (r.getDebugModuleStatus(DebugModule::Code))     r.renderCodeWidget();
    if (r.getDebugModuleStatus(DebugModule::Dma))      r.renderDmaWidget();
    if (r.getDebugModuleStatus(DebugModule::Timers))   r.renderTimersWidget();
    if (r.getDebugModuleStatus(DebugModule::Gpu))      r.renderGpuWidget();
    if (r.getDebugModuleStatus(DebugModule::Spu))      r.renderSpuWidget();
    if (r.getDebugModuleStatus(DebugModule::Cdrom))    r.renderCdromWidget();
    if (r.getDebugModuleStatus(DebugModule::Tty))      r.renderTtyWidget();
    
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

bool Debugger::renderFpsWidget()
{
    ImGui::SetNextWindowSize(ImVec2(100, 30));
    ImVec2 widgetPosition = ImGui::GetMainViewport()->WorkPos;
    widgetPosition.x += ImGui::GetMainViewport()->WorkSize.x;
    widgetPosition.y += ImGui::GetMainViewport()->WorkSize.y;
    ImGui::SetNextWindowPos(widgetPosition, 0, ImVec2(1.0f, 1.0f));
    
    if (stepMode == StepMode::Frame)
    {
        ImGui::Begin("FPS", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
        ImGui::Text("FPS: %.4d\t", framePerSecond);
        ImGui::End();
    }
    else
    {
        ImGui::Begin("FPS", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
        ImGui::Text("FPS: Stop\t");
        ImGui::End();
    }
    
    return true;
}

bool Debugger::renderBiosWidget()
{
    dbgRom.DrawWindow("BIOS", psx->bios->rom, BIOS_SIZE);
    return true;
}

bool Debugger::renderRamWidget()
{
    dbgRam.DrawWindow("RAM", psx->mem->ram, RAM_SIZE);
    return true;
}

bool Debugger::renderCpuWidget()
{
    //Update Interrupt Debug Info
    auto& r = instance();  // Alias al singleton
    r.getInterruptDebugInfo();

    ImGui::Begin("CPU Registers");
        ImGui::BeginTabBar("#tabs");
            if (ImGui::BeginTabItem("CPU"))
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
                    case 3:
                        ImGui::SameLine();
                        ImGui::Text("Clk Count = %d", psx->masterClock);
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
                    case 7:
                        ImGui::SameLine();
                        ImGui::Text("%s", (psx->cpu->cop0->reg[12] & 0x1) ? "Interrupt Enabled" : "Interrupt Disabled");
                        break;
                    case 8:
                        ImGui::SameLine();
                        ImGui::Text("%s", (psx->cpu->cop0->reg[12] & 0x2) ? "User Mode" : "Kernel Mode");
                        break;

                    case 10:
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

bool Debugger::renderCodeWidget()
{
    ImGui::Begin("Assembler");

    uint32_t addr = psx->cpu->pc;

    // Debug on-the-fly software in RAM
    // Check if code is already disassembled, if not disassemble starting from current Program Counter.
    if (asmCode.find(addr) == asmCode.end() || std::get<2>(asmCode[addr]) == "")
    {
        AsmCode newDisassembledCode = mipsDisassembler.disassemble(psx->bios->rom, psx->mem->ram, addr);
        for (auto& e : newDisassembledCode)
            asmCode.insert_or_assign(e.first, e.second);
    }

    addr -= (15 * 4);

    // Begin table with 3 columns: Label / Address / Assembler
    if (ImGui::BeginTable("AssemblerTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        // Setup headers
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("Assembler", ImGuiTableColumnFlags_WidthFixed, 2000.0f);
        ImGui::TableHeadersRow();

        for (int line = 0; line < 32; line++)
        {
            ImGui::TableNextRow();

            // Column 1: Label (empty)
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(""); // intentionally empty

            // Column 2: Address
            ImGui::TableSetColumnIndex(1);
            if (addr == psx->cpu->pc)
                ImGui::TextColored(white_color, "0x%08x", addr);
            else
                ImGui::TextColored(yellow_color, "0x%08x", addr);

            // Column 3: Assembler
            ImGui::TableSetColumnIndex(2);
            if (addr == psx->cpu->pc)
            {
                ImGui::TextColored(white_color, "0x%08x  %s  %s",
                    std::get<0>(asmCode[addr]),
                    std::get<1>(asmCode[addr]).c_str(),
                    std::get<2>(asmCode[addr]).c_str()
                );
            }
            else
            {
                ImGui::TextColored(green_color, "0x%08x", std::get<0>(asmCode[addr]));
                ImGui::SameLine();
                ImGui::TextColored(grey_color, "%s", std::get<1>(asmCode[addr]).c_str());
                ImGui::SameLine();
                ImGui::TextColored(grey_color, "%s", std::get<2>(asmCode[addr]).c_str());
            }

            addr += 4;
        }

        ImGui::EndTable();
    }

    ImGui::End();

    return true;
}

bool Debugger::renderDmaWidget()
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

bool Debugger::renderTimersWidget()
{
    //Update Timer Debug Info
    auto& r = instance();  // Alias al singleton
    r.getTimerDebugInfo();

    ImGui::Begin("TIMERS");

    const char* rowLabels[] = { "Mode", "Value", "Target", "Sync Mode", "Clock Source" };
    ImVec4 greenColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green for headers and row labels

    // Begin table with 4 columns: Row Label + Timer0/1/2
    if (ImGui::BeginTable("TimersTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        // Setup columns with fixed width 150
        ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Timer0", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Timer1", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Timer2", ImGuiTableColumnFlags_WidthFixed, 150.0f);

        // Table headers (green)
        ImGui::TableHeadersRow();
        for (int col = 0; col < 4; col++)
        {
            ImGui::TableSetColumnIndex(col);
            ImGui::TextColored(greenColor, "%s",
                col == 0 ? "Property" : (col == 1 ? "Timer0" : (col == 2 ? "Timer1" : "Timer2")));
        }

        // Fill rows
        for (int row = 0; row < 5; row++)
        {
            ImGui::TableNextRow();

            // First column = row label (green)
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(greenColor, "%s", rowLabels[row]);

            // Columns 1-3 = Timer0/1/2 values
            for (int timer = 0; timer < TIMER_NUMBER; timer++)
            {
                ImGui::TableSetColumnIndex(timer + 1);

                uint32_t mode = timerInfo.timerStatus[timer].counterMode.word;

                switch (row)
                {
                case 0: // Mode
                    ImGui::Text("0x%08x", mode);
                    break;
                case 1: // Value
                    ImGui::Text("0x%08x", timerInfo.timerStatus[timer].counterValue);
                    break;
                case 2: // Target
                    ImGui::Text("0x%08x", timerInfo.timerStatus[timer].counterTarget);
                    break;
                case 3: // Sync Mode
                {
                    bool syncEnable = (mode & 0x1) != 0;
                    uint32_t syncModeBits = (mode >> 1) & 0x3;
                    const char* syncText = "";

                    if (!syncEnable)
                    {
                        syncText = "Free Run";
                    }
                    else
                    {
                        switch (timer)
                        {
                        case 0: // Timer 0
                            switch (syncModeBits)
                            {
                            case 0: syncText = "Pause during Hblank(s)"; break;
                            case 1: syncText = "Reset 0000h at Hblank(s)"; break;
                            case 2: syncText = "Reset 0000h at Hblank(s), pause outside"; break;
                            case 3: syncText = "Pause until 1 Hblank, then Free Run"; break;
                            }
                            break;
                        case 1: // Timer 1
                            switch (syncModeBits)
                            {
                            case 0: syncText = "Pause during Vblank(s)"; break;
                            case 1: syncText = "Reset 0000h at Vblank(s)"; break;
                            case 2: syncText = "Reset 0000h at Vblank(s), pause outside"; break;
                            case 3: syncText = "Pause until 1 Vblank, then Free Run"; break;
                            }
                            break;
                        case 2: // Timer 2
                            if (syncModeBits == 0 || syncModeBits == 3)
                                syncText = "Stop counter at current value";
                            else
                                syncText = "Free Run (Sync disabled behavior)";
                            break;
                        }
                    }

                    ImGui::Text("%s", syncText);
                    break;
                }
                case 4: // Clock Source
                {
                    uint32_t clockBits = (mode >> 8) & 0x3;
                    const char* clockText = "";

                    switch (timer)
                    {
                    case 0:
                        clockText = (clockBits == 0 || clockBits == 2) ? "System Clock" : "Dotclock";
                        break;
                    case 1:
                        clockText = (clockBits == 0 || clockBits == 2) ? "System Clock" : "Hblank";
                        break;
                    case 2:
                        clockText = (clockBits == 0 || clockBits == 1) ? "System Clock" : "System Clock/8";
                        break;
                    }

                    ImGui::Text("%s", clockText);
                    break;
                }
                }
            }
        }

        ImGui::EndTable();
    }

    ImGui::End();

    return true;
}

bool Debugger::renderGpuWidget()
{
    //Update GPU Debug Info
    auto& r = instance();  // Alias al singleton
    r.getGpuDebugInfo();

    ImGui::Begin("GPU");

    // Create a table with 2 columns: Label / Value
    if (ImGui::BeginTable("GPU Info Table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Register", ImGuiTableColumnFlags_WidthFixed, 190.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 200.0f);

        // Row 1: GPUSTAT
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::PushStyleColor(ImGuiCol_Text, green_color); ImGui::Text("GPUSTAT"); ImGui::PopStyleColor();// Label
        ImGui::TableNextColumn(); ImGui::Text("0x%08x", gpuInfo.gpuStat); // Value

        // Row 2: Video Mode
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Text("Video Mode");
        ImGui::TableNextColumn(); ImGui::Text("%dx%d (%s - %s)", gpuInfo.videoResolution.x, gpuInfo.videoResolution.y, gpuInfo.videoStandard.c_str(), gpuInfo.interlaced ? "Interlaced" : "Progressive");

        // Row 3: Display Enabled
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Text("Display Enabled");
        ImGui::TableNextColumn(); ImGui::Text("%s", gpuInfo.displayDisabled ? "Disabled" : "Enabled");

        // Row 3: Display Start
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Text("Display Start");
        ImGui::TableNextColumn(); ImGui::Text("[%4d, %4d]", gpuInfo.displayStart.x, gpuInfo.displayStart.y);

        // Row 4: Display Horizontal Range
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Text("Display Horizontal Range");
        ImGui::TableNextColumn(); ImGui::Text("[%4d, %4d]", gpuInfo.displayRange.x1, gpuInfo.displayRange.x2);

        // Row 5: Display Vertical Range
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Text("Display Vertical Range");
        ImGui::TableNextColumn(); ImGui::Text("[%4d, %4d]", gpuInfo.displayRange.y1, gpuInfo.displayRange.y2);

        // Row 6: Drawing Area
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Text("Drawing Area");
        ImGui::TableNextColumn(); ImGui::Text("[%4d, %4d] - [%4d, %4d]", gpuInfo.drawingArea.x1, gpuInfo.drawingArea.y1, gpuInfo.drawingArea.x2, gpuInfo.drawingArea.y2);

        // Row 7: Drawing Offset
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Text("Drawing Offset");
        ImGui::TableNextColumn(); ImGui::Text("[%4d, %4d]", (int16_t)gpuInfo.drawingOffset.x, (int16_t)gpuInfo.drawingOffset.y);

        // Row 8: Texture Page
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Text("Texture Page");
        ImGui::TableNextColumn(); ImGui::Text("[%4d, %4d]", gpuInfo.texturePage.x, gpuInfo.texturePage.y);
        
        // Row 9: Texture Color Mode
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Text("Texture Color Mode");
        ImGui::TableNextColumn(); ImGui::Text("%s", gpuInfo.colorMode.c_str());      

        // Row 10: Texture Windows Mask
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Text("Texture Windows Mask");
        ImGui::TableNextColumn(); ImGui::Text("[%4d, %4d]", gpuInfo.textureMask.x, gpuInfo.textureMask.y);

        // Row 11: Texture Windows Offset
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Text("Texture Windows Offset");
        ImGui::TableNextColumn(); ImGui::Text("[%4d, %4d]", gpuInfo.textureOffset.x, gpuInfo.textureOffset.y);

        ImGui::EndTable(); // End of table
    }
    
    ImVec2 imageSize(1024, 512);
    ImVec2 imagePos = ImGui::GetCursorScreenPos(); // top-left corner
    
    ImGui::Image((void*)(intptr_t)Renderer::GetVRAMTextureObject(), imageSize);
    
    // Draw Image Border
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 bottomRight = ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y);

    // Disegna un rettangolo bordo esterno (color = RGBA)
    drawList->AddRect(imagePos, bottomRight, IM_COL32(60, 60, 60, 255), 1.0f, 0, 1.0f);
    ImGui::End();

    return true;      
}

bool Debugger::renderSpuWidget()
{
    return false;
}

bool Debugger::renderCdromWidget()
{
    //Update CDROM Debug Info
    auto& r = instance();  // Alias al singleton
    r.getCdromDebugInfo();

    ImGui::Begin("CDROM");

    // Status Register Table
    if (ImGui::BeginTable("Status Register", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV))
    {
        ImGui::TableSetupColumn("Field", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 150.0f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, green_color);
        ImGui::Text("Status Register");
        ImGui::PopStyleColor();
        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, green_color);
        ImGui::Text("0x%02x", cdromInfo.statusRegister);
        ImGui::PopStyleColor();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("ra");
        ImGui::TableNextColumn();
        ImGui::Text("%d", cdromInfo.statusRegister & 0x3);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("adpbusy");
        ImGui::TableNextColumn();
        ImGui::Text("%s", (cdromInfo.statusRegister & 0x4) ? "playing XA-ADPCM" : "Idle");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("prmempty");
        ImGui::TableNextColumn();
        ImGui::Text("%s", (cdromInfo.statusRegister & 0x8) ? "Params FIFO Empty" : "Params Available");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("prmwrdy");
        ImGui::TableNextColumn();
        ImGui::Text("%s", (cdromInfo.statusRegister & 0x10) ? "Param. FIFO writable" : "Param. FIFO Full");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("rslrrdy");
        ImGui::TableNextColumn();
        ImGui::Text("%s", (cdromInfo.statusRegister & 0x20) ? "Result Ready" : "No Result available");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("drqsts");
        ImGui::TableNextColumn();
        ImGui::Text("%s", (cdromInfo.statusRegister & 0x40) ? "R/W Request Pending" : "No R/W Request");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("busysts");
        ImGui::TableNextColumn();
        ImGui::Text("%s", (cdromInfo.statusRegister & 0x80) ? "Busy" : "Idle");

        ImGui::EndTable();
    }

    // Request Register Table
    if (ImGui::BeginTable("Request Register", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV))
    {
        ImGui::TableSetupColumn("Field", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 150.0f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, green_color);
        ImGui::Text("Request Register");
        ImGui::PopStyleColor();
        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, green_color);
        ImGui::Text("0x%02x", cdromInfo.requestRegister);
        ImGui::PopStyleColor();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("reserved");
        ImGui::TableNextColumn();
        ImGui::Text("%d", cdromInfo.requestRegister & 0x1f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("smen");
        ImGui::TableNextColumn();
        ImGui::Text("%s", (cdromInfo.requestRegister & 0x20) ? "Sound Map Enabled" : "Sound Map Disabled");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("bfwr");
        ImGui::TableNextColumn();
        ImGui::Text("%s", (cdromInfo.requestRegister & 0x40) ? "Writes to WRDATA" : "Idle");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("bfrd");
        ImGui::TableNextColumn();
        ImGui::Text("%s", (cdromInfo.requestRegister & 0x80) ? "Reads from RDDATA" : "Idle");

        ImGui::EndTable();
    }

    // Interrupt Mask Register Table
    if (ImGui::BeginTable("Interrupt Mask Register", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV))
    {
        ImGui::TableSetupColumn("Field", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 150.0f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, green_color);
        ImGui::Text("Interrupt Mask");
        ImGui::PopStyleColor();
        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, green_color);
        ImGui::Text("0x%02x", cdromInfo.interruptMaskRegister);
        ImGui::PopStyleColor();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("enint");
        ImGui::TableNextColumn();
        ImGui::Text("%s", (cdromInfo.interruptMaskRegister & 0x07) ? "Enabled" : "Disabled");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("enbfempt");
        ImGui::TableNextColumn();
        ImGui::Text("%s", (cdromInfo.interruptMaskRegister & 0x08) ? "Enabled" : "Disabled");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("enbfwrdy");
        ImGui::TableNextColumn();
        ImGui::Text("%s", (cdromInfo.interruptMaskRegister & 0x10) ? "Enabled" : "Disabled");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("reserved");
        ImGui::TableNextColumn();
        ImGui::Text("%d", cdromInfo.interruptMaskRegister & 0xe0);

        ImGui::EndTable();
    }

    // Interrupt Status Register Table
    if (ImGui::BeginTable("Interrupt Status Register", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV))
    {
        ImGui::TableSetupColumn("Field", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 150.0f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, green_color);
        ImGui::Text("Interrupt Status");
        ImGui::PopStyleColor();
        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, green_color);
        ImGui::Text("0x%02x", cdromInfo.interruptStatusRegister);
        ImGui::PopStyleColor();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("intsts");
        ImGui::TableNextColumn();
        char buf[16];
        sprintf(buf, "INT%d", cdromInfo.interruptStatusRegister & 0x07);
        ImGui::Text("%s", (cdromInfo.interruptStatusRegister & 0x07) ? buf : "-");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("bfempt");
        ImGui::TableNextColumn();
        ImGui::Text("%s", (cdromInfo.interruptStatusRegister & 0x08) ? "XA-ADPCM buffer empty" : "-");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("bfwrdy");
        ImGui::TableNextColumn();
        ImGui::Text("%s", (cdromInfo.interruptStatusRegister & 0x10) ? "XA-ADPCM buffer write ready" : "-");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("reserved");
        ImGui::TableNextColumn();
        ImGui::Text("%d", cdromInfo.interruptStatusRegister & 0xe0);

        ImGui::EndTable();
    }

    ImGui::End();
    return true;
}

bool Debugger::renderTtyWidget()
{
    ImGui::Begin("TTY");
    for (std::string s : psx->tty->bufferA)
        ImGui::Text("%s", s.c_str());
    ImGui::SetScrollHereY(1.0f);
    ImGui::End();
    return true;
}

bool Debugger::renderMenuBar()
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
                openSetBreakpointPopup = true;
            }
            if (ImGui::MenuItem("Conditions..."))
            {
                //TODO
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();

        // Open popup after menu closed (works around menu focus)
        if (openSetBreakpointPopup)
        {
            ImGui::OpenPopup("Set Breakpoint");
            openSetBreakpointPopup = false;
        }

        // Set Breakpoint popup
        if (ImGui::BeginPopupModal("Set Breakpoint", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char breakpointInput[64];
            static bool bp_init = false;
            if (!bp_init)
            {
                sprintf(breakpointInput, "0x%08x", breakPoint);
                bp_init = true;
            }

            ImGui::Text("Enter breakpoint address (hex):");
            ImGui::InputText("##breakpoint", breakpointInput, IM_ARRAYSIZE(breakpointInput));

            if (ImGui::Button("OK"))
            {
                // Parse hex (allow 0x prefix or plain hex)
                char* endptr = nullptr;
                unsigned long val = strtoul(breakpointInput, &endptr, 0);
                if (endptr != breakpointInput)
                {
                    setBreakpoint(static_cast<uint32_t>(val));
                }
                bp_init = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
            {
                bp_init = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    return true;
}

void Debugger::dumpRam()
{
    std::ofstream fs("ramdump.bin", std::ios::out | std::ios::binary);
    fs.write(reinterpret_cast<const char*>(instance().psx->mem->ram), 0x200000);
    fs.close();
}

//------------------------------------------------------------------------------------------
// Debug Info Update Functions
//------------------------------------------------------------------------------------------
void Debugger::getInterruptDebugInfo()
{
    interruptInfo.i_mask = psx->interrupt->getMaskRegister();
    interruptInfo.i_stat = psx->interrupt->getStatusRegister();
}

void Debugger::getTimerDebugInfo()
{
    timerInfo.timerStatus[0] = psx->timers->getTimerStatus(0);
    timerInfo.timerStatus[1] = psx->timers->getTimerStatus(1);
    timerInfo.timerStatus[2] = psx->timers->getTimerStatus(2);
}

void Debugger::getGpuDebugInfo()
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
    gpuInfo.colorMode = psx->gpu->getTextureColorDepth();
    gpuInfo.videoStandard = psx->gpu->getVideoStandard();
    gpuInfo.interlaced = psx->gpu->getVerticalInterlace();
    gpuInfo.displayDisabled = psx->gpu->getDisplayDisabled();
}

void Debugger::getCdromDebugInfo()
{
    cdromInfo.statusRegister = psx->cdrom->getStatusRegister();
    cdromInfo.requestRegister = psx->cdrom->getRequestRegister();
    cdromInfo.interruptMaskRegister = psx->cdrom->getInterruptMaskRegister();
    cdromInfo.interruptStatusRegister = psx->cdrom->getInterruptStatusRegister();   
}

void Debugger::getCallStackInfo(KernelCallEvent &e)
{

    CallStackInfo callInfo;
    callInfo.pc = e.pc;   //Program Counter
    callInfo.sp = e.sp;   //Stack Pointer
    callInfo.ra = e.ra;   //Return Address

    //Generate Generic Function Name    
    std::stringstream functionName;
    //functionName << "function_" << std::hex << callInfo.pc;
    //callInfo.func = functionName.str();

    //A Functions Name Resolution
    if (callInfo.pc == 0x000000a0)
    {
        //Skip putchar function
        if (e.t1 == 0x3c) 
            return;
        
        functionName.str("");
        functionName << function_A[e.t1];
        LOG_F(2, "KRN - Calling: %s [a0: 0x%08x, a1: 0x%08x, a2: 0x%08x, a3: 0x%08x]", functionName.str().c_str(), e.a0, e.a1, e.a2, e.a3);
        callInfo.func = functionName.str();
        
        callStack.write(callInfo);
        return;
    }

    //B Functions Name Resolution
    if (callInfo.pc == 0x000000b0)
    {
        //Skip putchar function
        if (e.t1 == 0x3d) 
            return;

        functionName.str("");
        functionName << function_B[e.t1];
        LOG_F(2, "KRN - Calling: %s [a0: 0x%08x, a1: 0x%08x, a2: 0x%08x, a3: 0x%08x]", functionName.str().c_str(), e.a0, e.a1, e.a2, e.a3);
        callInfo.func = functionName.str();

        callStack.write(callInfo);
        return;
    }

    //C Functions Name Resolution
    if (callInfo.pc == 0x000000c0)
    {
        functionName.str("");
        functionName << function_C[e.t1];
        LOG_F(2, "KRN - Calling: %s [a0: 0x%08x, a1: 0x%08x, a2: 0x%08x, a3: 0x%08x]", functionName.str().c_str(), e.a0, e.a1, e.a2, e.a3);
        callInfo.func = functionName.str();

        callStack.write(callInfo);
        return;
    }

    return;
}