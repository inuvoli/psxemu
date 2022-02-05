#version 460 core

in vec3 vColor;
in vec2 vTextureCoords;
in vec2 clutTable;
in vec2 texturePage;
in float textureColorDepth;
in float textureOn;
// in float transpOn;
// in float blendingOn;

out vec4    fragColor;

uniform usampler2DRect videoRam;

vec4 getColor(int data)
{
    vec4 color;

    color.r = float(data & 0x1f) / 31.0;
    color.g = float((data >> 5) & 0x1f) / 31.0;
    color.b = float((data >> 10) & 0x1f) / 31.0;
    color.a = float((data >> 15) & 0x01);  

    return color;
}

int getTexel(int colorMode)
{
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
    texelCoord.x = texturePage.x + (vTextureCoords.x / texelPerHalfword);
    texelCoord.y = texturePage.y + (vTextureCoords.y);
    
    int value = int(texture(videoRam, texelCoord).r);

    return value;
}

int getClut4Entry(int texelData)
{
    int shiftValue = 3 - ((int(vTextureCoords.x) % 4) * 4);
    int index = int(texelData >> shiftValue) & 0x0f;
    
    vec2 clutPosition = vec2(clutTable.x + index, clutTable.y);
    int clutEntry = int(texture(videoRam, clutPosition).r);

    return clutEntry;
}

int getClut8Entry(int texelData)
{
    int shiftValue = ((int(vTextureCoords.x) % 2) * 8);
    int index = int(texelData >> shiftValue) & 0x00ff;
    
    vec2 clutPosition = vec2(clutTable.x + index, clutTable.y);
    int clutEntry = int(texture(videoRam, clutPosition).r);

    return clutEntry;
}

void main()
{
    if (textureOn == 1.0f)
    {
        int colorMode = int(textureColorDepth);     
        int texelData = getTexel(colorMode);

        vec4 texelColor;
        int clutEntry;

        switch (colorMode)
        {
            case 0:     //4 bit CLUT
                clutEntry = getClut4Entry(texelData);
                texelColor = getColor(clutEntry);
                fragColor.xyz = vColor.xyz * texelColor.xyz;
                fragColor.w = texelColor.w;
                break;
            case 1:     //8 bit CLUT
                clutEntry = getClut8Entry(texelData);
                texelColor = getColor(clutEntry);
                fragColor.xyz = vColor.xyz * texelColor.xyz;
                fragColor.w = texelColor.w;
                break;
            case 2:     //15 Bit Texel (5-5-5-1)
                texelColor = getColor(texelData);
                fragColor.xyz = vColor.xyz * texelColor.xyz;
                fragColor.w = texelColor.w;
                break;
            case 3:     //Reserved
                fragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
                break;
        }
        
    }
    else
    {
        fragColor.xyz = vColor;
        fragColor.w = 1.0f;
    }
   
    // fragColor.xyz = vColor;
    // fragColor.w = 1.0f;
}