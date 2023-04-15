#pragma once

//SDL specific Includes, note PSXEMU is using GLEW OpenGL loader
#include <GL/glew.h>
#include <SDL.h>

//ImGui specific Includes, note ImGui is using a stripped GL3W OpenGL loader
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "imgui_memory_editor.h"

// OpenGL 4.6 + GLSL v. 460
constexpr auto glsl_version = "#version 460 core";
constexpr auto opengl_major_version = 4;
constexpr auto opengl_minor_version = 6;