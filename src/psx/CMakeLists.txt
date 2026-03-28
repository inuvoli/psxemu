find_package(SDL3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(loguru CONFIG REQUIRED)

# Create a sources variable with a link to all cpp files to compile
set(PSXEMU_SOURCES
    memory/bios.cpp
	memory/dma.cpp
	memory/dmachannel.cpp
	memory/memory.cpp
	memory/exefile.cpp
    peripherals/cdrom.cpp
	peripherals/controller.cpp
	peripherals/interrupt.cpp
	cpu/cpu_short_pipe.cpp
	cpu/cop0.cpp
	cpu/cop2.cpp
	gpu/gpu.cpp
	gpu/renderer.cpp
	gpu/shader.cpp
	spu/spu.cpp
	timers/timers.cpp
	debugging/debugger.cpp
	debugging/mipsdisassembler.cpp
	debugging/tty.cpp
	psxemu/psxemu.cpp
	utils/commandline.cpp
	main.cpp
	psx.cpp
)

# IMGUI cpp files
set(IMGUI_SOURCES
	${PROJECT_SOURCE_DIR}/3rdparty/imgui/imgui.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/imgui/imgui_draw.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/imgui/imgui_tables.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/imgui/imgui_widgets.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_opengl3.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_sdl3.cpp
)

#  LIBCDIMAGE cpp files
set(LIBCDIMAGE_SOURCES
	${PROJECT_SOURCE_DIR}/3rdparty/libcdimage/libcdimage.cpp
)

# Add an executable with the above sources
add_executable(psxemu ${PSXEMU_SOURCES} ${IMGUI_SOURCES} ${LIBCDIMAGE_SOURCES})

# Set the directories that should be included in the build command for this target
# when running g++ these will be included as -I/directory/path/
target_include_directories(psxemu
    PRIVATE 
		${PROJECT_SOURCE_DIR}/3rdparty
		${PROJECT_SOURCE_DIR}/3rdparty/imgui
		${PROJECT_SOURCE_DIR}/3rdparty/imgui/backends
		${PROJECT_SOURCE_DIR}/3rdparty/imgui_club/imgui_memory_editor
		${PROJECT_SOURCE_DIR}/3rdparty/litelib
		${PROJECT_SOURCE_DIR}/3rdparty/libcdimage
		${PROJECT_SOURCE_DIR}/3rdparty/fpm/include
		${PROJECT_SOURCE_DIR}/src/psx
		${PROJECT_SOURCE_DIR}/src/psx/cpu
		${PROJECT_SOURCE_DIR}/src/psx/core
		${PROJECT_SOURCE_DIR}/src/psx/debugging
		${PROJECT_SOURCE_DIR}/src/psx/gpu
		${PROJECT_SOURCE_DIR}/src/psx/memory
		${PROJECT_SOURCE_DIR}/src/psx/peripherals
		${PROJECT_SOURCE_DIR}/src/psx/spu
		${PROJECT_SOURCE_DIR}/src/psx/timers
		${PROJECT_SOURCE_DIR}/src/psx/psxemu
		${PROJECT_SOURCE_DIR}/src/psx/utils
		${SDL3_INCLUDE_DIRS}
		${GLEW_INCLUDE_DIRS}
		${OPENGL_INCLUDE_DIRS}
		${GLM_INCLUDE_DIRS}
		${ZLIB_INCLUDE_DIRS}
		${LOGURU_INCLUDE_DIRS}
)

# Set libraries to be included
target_link_libraries(psxemu 
						PRIVATE 
							OpenGL::GL
							glm::glm
							GLEW::GLEW
							ZLIB::ZLIB
							SDL3::SDL3
							loguru::loguru
)
