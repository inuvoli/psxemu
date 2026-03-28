#version 460 core

// ------------------------------------------------------------
// Vertex inputs (from persistent mapped VBO)
// ------------------------------------------------------------

layout(location = 0) in vec2 aPosition;  // PSX pixel coords (integer stored as float)
layout(location = 1) in vec3 aColor;     // Vertex color (mapped from 0..255)
layout(location = 2) in vec2 aUV;        // Texture UV in VRAM space (pixel coords)


// ------------------------------------------------------------
// Uniforms (PSX GPU state)
// ------------------------------------------------------------

// Drawing offset register
uniform ivec2 uDrawOffset;

// Drawing area (clip rectangle)
//uniform ivec4 uDrawArea; // x0, y0, x1, y1 NOT USED HERE

// ------------------------------------------------------------
// Outputs to fragment shader
// ------------------------------------------------------------

out vec2 vUV;
out vec3 vColor;
out vec2 vFragPos; // Pixel position after offset (used for mask/dither)


// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------
void main()
{
    // --------------------------------------------------------
    // Apply drawing offset (PSX register)
    // --------------------------------------------------------
    ivec2 pos = ivec2(aPosition) + uDrawOffset;

    // --------------------------------------------------------
    // Convert to normalized device coordinates
    // PSX origin: top-left
    // OpenGL origin: center, Y up
    // --------------------------------------------------------
    vec2 ndc;
    ndc.x = (float(pos.x) / 1024.0) * 2.0 - 1.0;
    ndc.y = (float(pos.y) / 512.0) * 2.0 - 1.0;

    gl_Position = vec4(ndc, 0.0, 1.0);

    // --------------------------------------------------------
    // Pass-through attributes
    // --------------------------------------------------------
    vUV       = aUV;
    //vColor    = aColor / 255.0;
    vColor = floor((aColor * (31.0 / 255.0)) + 0.5);
    vFragPos  = vec2(pos.x, pos.y);
}
