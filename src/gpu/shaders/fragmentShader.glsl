#version 460 core

in vec3 vertexColor;
in vec2 textureCoord;

out vec4 fragColor;

uniform usampler2DRect videoRam;

uniform int textured;
uniform uint clutInfo;
uniform uint texPageInfo;

vec4 getColor(uint data)
{
    vec4 color;

    color.r = float(int(data) & 0x1f) / 31.0;
    color.g = float((int(data) >> 5) & 0x1f) / 31.0;
    color.b = float((int(data) >> 10) & 0x1f) / 31.0;
    color.a = float((int(data) >> 15) & 0x01);  

    return color;
}

uint getTexel(uint colorMode)
{
    vec2 texturePage;
    texturePage.x = float(((int(texPageInfo) & 0x0f)) * 64);
    texturePage.y = float((int(texPageInfo >> 4) & 0x01) * 256);

    int texelPerHalfword;
    switch (colorMode)
    {
        case 0:         //4 bit CLUT
            texelPerHalfword = 4;
            break;
        case 1:         //8 bit CLUT
            texelPerHalfword = 2;
            break;
        case 2:         //16bit Texel (5-5-5-1)
            texelPerHalfword = 1;
            break;
        case 3:         //Reserved
            texelPerHalfword = 1;
    }

    vec2 texelCoord;
    texelCoord.x = texturePage.x + int(textureCoord.x) / texelPerHalfword;
    texelCoord.y = texturePage.y + textureCoord.y;
    
    uint value = uint(texture(videoRam, texelCoord).r);

    return value;
}

uint getClut4Entry(uint texelData)
{
    int shiftValue = 3 - ((int(textureCoord.x) % 4) * 4);
    int index = int(texelData >> shiftValue) & 0x0f;

    vec2 clutCoord;
    clutCoord.x = float((int(clutInfo) & 0x001f) + index + 0.5f);
    clutCoord.y = float(int(clutInfo >> 6) & 0x01ff);
    
    uint clutEntry = uint(texture(videoRam, clutCoord).r);

    return clutEntry;
}

uint getClut8Entry(uint texelData)
{
    int shiftValue = (int(textureCoord.x) % 2) * 8;
    int index = int(texelData >> shiftValue) & 0x0f;

    vec2 clutCoord;
    clutCoord.x = float((int(clutInfo) & 0x001f) + index + 0.5f);
    clutCoord.y = float(int(clutInfo >> 6) & 0x01ff);
    
    uint clutEntry = uint(texture(videoRam, clutCoord));

    return clutEntry;
}

void main()
{
    if (textured == 1)
    {
        uint colorMode = uint(int(texPageInfo >> 7) & 0x03);
        uint texelData = getTexel(colorMode);

        vec4 texelColor;
        uint clutEntry;

        switch (colorMode)
        {
            case 0:     //4 bit CLUT
                clutEntry = getClut4Entry(texelData);
                texelColor = getColor(clutEntry);
                fragColor.xyz = vertexColor.xyz * texelColor.xyz;
                fragColor.w = texelColor.w;
                break;
            case 1:     //8 bit CLUT
                clutEntry = getClut8Entry(texelData);
                texelColor = getColor(clutEntry);
                fragColor.xyz = vertexColor.xyz * texelColor.xyz;
                fragColor.w + texelColor.w;
                break;
            case 2:     //15 Bit Texel (5-5-5-1)
                texelColor = getColor(texelData);
                fragColor.xyz = vertexColor.xyz * texelColor.xyz;
                fragColor.w + texelColor.w;
                break;
            case 3:     //Reserved
                fragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
                break;
        }
        
    }
    else
    {
        fragColor.xyz = vertexColor;
        fragColor.w = 1.0f;
    }   
}