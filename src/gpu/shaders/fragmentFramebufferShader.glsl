#version 460 core

in vec2 vUV;                 // [0..1] fullscreen quad
out vec4 FragColor;

uniform usampler2D uVRAM;    // GL_R16UI 1024×512

// Display parameters (PSX registers)
uniform ivec4 uDisplayArea;  // x1, y1, width, height
uniform int   uColorMode;    // 0 = 15 bit, 1=24bit NOT USED YET

// Video mode
uniform bool uInterlaced;    // true = interlaced, false = progressive
uniform bool uEvenField;     // current field (true = even, false = odd)

// ------------------------------------------------------------
// RGB555 → RGB888 conversion (PSX DAC behavior)
// ------------------------------------------------------------
vec3 decodeRGB555(uint c)
{
    uint r5 = (c >>  0) & 31u;
    uint g5 = (c >>  5) & 31u;
    uint b5 = (c >> 10) & 31u;

    // Expand 5 → 8 bits (bit replication, closer to DAC)
    uint r8 = (r5 << 3) | (r5 >> 2);
    uint g8 = (g5 << 3) | (g5 >> 2);
    uint b8 = (b5 << 3) | (b5 >> 2);

    return vec3(r8, g8, b8) / 255.0;
}

// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------
void main()
{
    // Screen-space pixel inside the display area
    int sx = int(vUV.x * float(uDisplayArea.z));
    int sy = int(vUV.y * float(uDisplayArea.w));

    // Interlacing (PSX-style)
    if (uInterlaced)
    {
        // Force even or odd field
        sy = (sy & ~1) + (uEvenField ? 0 : 1);
    }

    // Final VRAM coordinates
    ivec2 vramPos = ivec2(
        uDisplayArea.x + sx,
        uDisplayArea.y + sy
    );

    // Clamp defensively (some games rely on overscan)
    vramPos.x = clamp(vramPos.x, 0, 1023);
    vramPos.y = clamp(vramPos.y, 0, 511);

    // Fetch raw RGB555 pixel
    uint raw = texelFetch(uVRAM, vramPos, 0).r;

    // Convert and output
    FragColor = vec4(decodeRGB555(raw), 1.0);
}
