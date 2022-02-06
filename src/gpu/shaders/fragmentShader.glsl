#version 460 core

in vec3 vColor;
in vec2 vTextureCoords;
in vec2 clutTable;
in float textureOn;
in vec2 texturePage;
in float textureColorDepth;
in float textureBlending;
in float transOn;
in float transAlgo;

out vec4    fragColor;

uniform usampler2DRect videoRam;

vec4 decodePsxColor(int data)
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

int getClutColor(int texelData, int colorDepth)
{
    int texelPerHalfWord = 16 / colorDepth;
    int texelMask = 0xffff >> (16 - colorDepth);
    
    int shiftValue = ((int(vTextureCoords.x) % texelPerHalfWord) * colorDepth);
    int index = (texelData >> shiftValue) & texelMask;
    
    vec2 clutPosition = vec2(clutTable.x + float(index), clutTable.y);
    int clutColor = int(texture(videoRam, clutPosition).r);

    return clutColor;
}

void main()
{
    if (textureOn == 1.0f)
    {
        int colorMode = int(textureColorDepth);     
        int texelData = getTexel(colorMode);

        int clutColor;
        vec4 finalTexelColor;

        switch (colorMode)
        {
            case 0:     //4 bit CLUT
                clutColor = getClutColor(texelData, 4);
                finalTexelColor = decodePsxColor(clutColor); 
                break;
            case 1:     //8 bit CLUT
                clutColor = getClutColor(texelData, 8);
                finalTexelColor = decodePsxColor(clutColor);
                break;
            case 2:     //15 Bit Texel (5-5-5-1)
                finalTexelColor = decodePsxColor(texelData);
                break;
            case 3:     //Reserved, same as 15Bit Texel
                finalTexelColor = decodePsxColor(texelData);
                break;
        }

        //Color 0000h is full transparent for PSX palette
        if (finalTexelColor == vec4(0.0f,0.0f,0.0f,0.0f))
                discard;

        if (textureBlending == 1.0f)
        {
            
                

            fragColor.xyz = (finalTexelColor.xyz); //TODO Texture Shading
            fragColor.w = 1.0f;
            //fragColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);
        }
        else
        {
            //fragColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
            fragColor.xyz = finalTexelColor.xyz;
            fragColor.w = 1.0f;
        }
    }
    else
    {
        fragColor.xyz = vColor;
        fragColor.w = 1.0f;
    }
}