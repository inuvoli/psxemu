#version 460 core

// ------------------------------------------------------------
// Inputs from vertex shader
// ------------------------------------------------------------

in vec2 vUV;          // texture coordinates in VRAM pixel space [0,0 - 1024, 512]
in vec3 vColor;       // vertex color in RGB5 format
in vec2 vFragPos;     // pixel position in VRAM pixel space (after draw offset)

// ------------------------------------------------------------
// Output to VRAM texture
// ------------------------------------------------------------

layout(location = 0) out uvec4 outColor;   // RGB555 + mask bit

// ------------------------------------------------------------
// Uniforms (PSX GPU state)
// ------------------------------------------------------------

// VRAM as integer texture (GL_R16UI)
uniform usampler2D uVRAM;

// Drawing area (clip rectangle)
uniform ivec4 uDrawArea;            // x0, y0, x1, y1
uniform bool  uDrawEnable;          // Set by renderer but not used yet, enable/disable writing inside Drawing Area    

// Texture page and CLUT
uniform ivec2 uTPage;               // texture page base (x, y)
uniform ivec2 uClut;                // CLUT base (x, y)
uniform int   uTexColorMode;        // 4, 8, or 16
uniform ivec2 uTexMask;             // Set by renderer but not used yet
uniform ivec2 uTexOffset;           // Set by renderer but not used yet

// Texture flags
uniform bool uTextured;
uniform bool uTexBlending;         

// Semi-transparency
uniform bool uSemiTrans;
uniform int  uSemiTransMode;        

// Mask bit
uniform bool uCheckMask;
uniform bool uForceMask;

// Dithering
uniform bool uDither;

// ------------------------------------------------------------
// PSX dithering matrix (4x4)
// ------------------------------------------------------------

const int dither[16] = int[16](
    -4,  0, -3,  1,
     2, -2,  3, -1,
    -3,  1, -4,  0,
     3, -1,  2, -2
);

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

uvec3 unpack555(uint c)
{
    return uvec3(
        (c >>  0) & 31u,    //Red
        (c >>  5) & 31u,    //Green
        (c >> 10) & 31u     //Blue
    );
}

uint pack555(uvec3 c, bool mask)
{
    return
        (c.r & 31u) |           //Red
        ((c.g & 31u) << 5) |    //Green
        ((c.b & 31u) << 10) |   //Blue
        (mask ? 0x8000u : 0u);
}

// ------------------------------------------------------------
// Texture fetch
// ------------------------------------------------------------

uint fetchTexel(ivec2 uv)
{
    uvec2 p;
    if (uTexColorMode == 16)            //Raw 1-5-5-5 format
    {
        p = uTPage + uv;
        return texelFetch(uVRAM, ivec2(p.x, p.y), 0).r;
    }
    else if (uTexColorMode == 8)        //CLUT 8bit
    {
        p = uTPage + uvec2(uv.x / 2, uv.y);
        uint w = texelFetch(uVRAM, ivec2(p.x, p.y), 0).r;
        uint idx = (uv.x & 1) == 0 ? (w & 0xffu) : (w >> 8);
        p = uClut + uvec2(int(idx), 0);
        return texelFetch(uVRAM, ivec2(p.x, p.y), 0).r;
    }
    else                                //CLUT 4bit
    {
        p = uTPage + ivec2(uv.x / 4, uv.y);
        uint w = texelFetch(uVRAM, ivec2(p.x, p.y), 0).r;
        uint shift = uint((uv.x & 3) * 4);
        uint idx = (w >> shift) & 0x0fu;
        p = uClut + uvec2(int(idx), 0);
        return texelFetch(uVRAM, ivec2(p.x, p.y), 0).r;
    }
}

// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------

void main()
{
    uvec2 pos = uvec2(vFragPos);
    uvec3 color = uvec3(vColor);

    // --------------------------------------------------------
    // Drawing area clipping (pixel-accurate)
    // --------------------------------------------------------
    if (pos.x < uDrawArea.x || pos.x > uDrawArea.z ||
        pos.y < uDrawArea.y || pos.y > uDrawArea.w)
        discard;

    // --------------------------------------------------------
    // Background pixel (for blending & mask test)
    // --------------------------------------------------------
    uint dstRaw = texelFetch(uVRAM, ivec2(pos.x, pos.y), 0).r;

    if (uCheckMask && ((dstRaw & 0x8000u) != 0u))
        discard;

    uvec3 dst = unpack555(dstRaw);

    // --------------------------------------------------------
    // Source color
    // --------------------------------------------------------
    uvec3 src = color;

    if (uTextured)
    {
        uint texel = fetchTexel(ivec2(vUV));

        // Texture transparency (index 0)
        if ((texel & 0x7fffu) == 0u)
           discard;

        src = unpack555(texel);

        // Texture blending
        if (uTexBlending)
        {
            src = (src * color) / 31u;
        }
    }
    else
    {
        src = color;
    }

    // --------------------------------------------------------
    // Semi-transparency
    // --------------------------------------------------------
    if (uSemiTrans)
    {
        if (uSemiTransMode == 0)
            src = (src + dst) >> 1;
        else if (uSemiTransMode == 1)
            src = min(src + dst, uvec3(31));
        else if (uSemiTransMode == 2)
            src = max(src - dst, uvec3(0));
        else
            src = min(src + (dst >> 1), uvec3(31));
    }

    // --------------------------------------------------------
    // Dithering
    // --------------------------------------------------------
    if (uDither)
    {
        int d = dither[int(pos.y & 3) * 4 + int(pos.x & 3)];
        uvec3 src8 = (src << 3) | (src >> 2);
        src = uvec3(
            clamp(int(src8.r) + d, 0, 255),
            clamp(int(src8.g) + d, 0, 255),
            clamp(int(src8.b) + d, 0, 255)
        );
        src = src >> 3;
    }

    // --------------------------------------------------------
    // Write mask bit
    // --------------------------------------------------------
    bool mask = uForceMask; 

    outColor = uvec4(pack555(src, mask), 0u, 0u, 1u);
}