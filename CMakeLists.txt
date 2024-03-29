# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version
cmake_minimum_required(VERSION 3.5)

# Set the project name
project (psxemu VERSION 0.1)

# Set Policies
cmake_policy(SET CMP0072 NEW)

# Set variables
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)

find_package(SDL2 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

# Create a sources variable with a link to all cpp files to compile
set(PSXEMU_SOURCES
    src/memory/bios.cpp
	src/memory/dma.cpp
	src/memory/dmachannel.cpp
	src/memory/memory.cpp
    src/peripherals/cdrom.cpp
	src/peripherals/controller.cpp
	src/peripherals/interrupt.cpp
	src/cpu/cpu_short_pipe.cpp
	src/cpu/cop0.cpp
	src/cpu/cop2.cpp
	src/gpu/gpu.cpp
	src/gpu/renderer.cpp
	src/gpu/shader.cpp
	src/spu/spu.cpp
	src/timers/timers.cpp
	src/debugging/debugger.cpp
	src/debugging/mipsdisassembler.cpp
	src/debugging/tty.cpp
	src/main.cpp
	src/psx.cpp
	src/psxemu.cpp
)

# IMGUI cpp files
set(IMGUI_SOURCES
	3rdparty/imgui/imgui.cpp
	3rdparty/imgui/imgui_draw.cpp
	3rdparty/imgui/imgui_impl_opengl3.cpp
	3rdparty/imgui/imgui_impl_sdl2.cpp
	3rdparty/imgui/imgui_tables.cpp
	3rdparty/imgui/imgui_widgets.cpp
)

#  LOGURU cpp files
set(LOGURU_SOURCES
	3rdparty/loguru/loguru.cpp
)

#  LIBCDIMAGE cpp files
set(LIBCDIMAGE_SOURCES
	3rdparty/libcdimage/libcdimage.cpp
)

# Add an executable with the above sources
add_executable(psxemu ${PSXEMU_SOURCES} ${IMGUI_SOURCES} ${LOGURU_SOURCES} ${LIBCDIMAGE_SOURCES})

# Set the directories that should be included in the build command for this target
# when running g++ these will be included as -I/directory/path/
target_include_directories(psxemu
    PRIVATE 
		${PROJECT_SOURCE_DIR}/3rdparty
		${PROJECT_SOURCE_DIR}/3rdparty/imgui
		${PROJECT_SOURCE_DIR}/3rdparty/litelib
		${PROJECT_SOURCE_DIR}/3rdparty/loguru
		${PROJECT_SOURCE_DIR}/3rdparty/libcdimage
		${PROJECT_SOURCE_DIR}/src
		${PROJECT_SOURCE_DIR}/src/cpu
		${PROJECT_SOURCE_DIR}/src/core
		${PROJECT_SOURCE_DIR}/src/debugging
		${PROJECT_SOURCE_DIR}/src/gpu
		${PROJECT_SOURCE_DIR}/src/memory
		${PROJECT_SOURCE_DIR}/src/peripherals
		${PROJECT_SOURCE_DIR}/src/spu
		${PROJECT_SOURCE_DIR}/src/timers
		${SDL2_INCLUDE_DIRS}
		${GLEW_INCLUDE_DIRS}
		${OPENGL_INCLUDE_DIRS}
		${GLM_INCLUDE_DIRS}
		${ZLIB_INCLUDE_DIRS}
)

# Set libraries to be included
target_link_libraries(psxemu 
						PRIVATE 
							OpenGL::GL
							glm::glm
							GLEW::GLEW
							ZLIB::ZLIB
							$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        					$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
