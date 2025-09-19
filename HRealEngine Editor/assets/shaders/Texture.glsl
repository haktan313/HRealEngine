//Texture.glsl
#type vertex
#version 450 core

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec4 v_Color;
layout(location = 2) in vec2 v_texCoordIn;
layout(location = 3) in float v_TexIndex;
layout(location = 4) in float v_TilingFactor;
layout(location = 5) in int v_EntityID;

uniform mat4 u_ViewProjectionMatrix;

out vec4 v_ColorOut;
out vec2 v_texCoordOut;
out float v_TilingFactorOut;

out flat float v_TexIndexOut;
out flat int v_EntityIDOut;

void main()
{
    v_ColorOut = v_Color;
    v_texCoordOut = v_texCoordIn;
    v_TilingFactorOut = v_TilingFactor;

    v_TexIndexOut = v_TexIndex;
    v_EntityIDOut = v_EntityID;

    gl_Position = u_ViewProjectionMatrix * vec4(v_Position, 1.0f);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 v_color;
layout(location = 1) out int objectID;

in vec4 v_ColorOut;
in vec2 v_texCoordOut;
in float v_TilingFactorOut;
in flat float v_TexIndexOut;
in flat int v_EntityIDOut;

uniform sampler2D u_textureSamplers[32];

void main()
{
    vec4 texColor = v_ColorOut;

    switch(int(v_TexIndexOut))
    {
        case 0: texColor *= texture(u_textureSamplers[0], v_texCoordOut * v_TilingFactorOut); break;
        case 1: texColor *= texture(u_textureSamplers[1], v_texCoordOut * v_TilingFactorOut); break;
        case 2: texColor *= texture(u_textureSamplers[2], v_texCoordOut * v_TilingFactorOut); break;
        case 3: texColor *= texture(u_textureSamplers[3], v_texCoordOut * v_TilingFactorOut); break;
        case 4: texColor *= texture(u_textureSamplers[4], v_texCoordOut * v_TilingFactorOut); break;
        case 5: texColor *= texture(u_textureSamplers[5], v_texCoordOut * v_TilingFactorOut); break;
        case 6: texColor *= texture(u_textureSamplers[6], v_texCoordOut * v_TilingFactorOut); break;
        case 7: texColor *= texture(u_textureSamplers[7], v_texCoordOut * v_TilingFactorOut); break;
        case 8: texColor *= texture(u_textureSamplers[8], v_texCoordOut * v_TilingFactorOut); break;
        case 9: texColor *= texture(u_textureSamplers[9], v_texCoordOut * v_TilingFactorOut); break;
        case 10: texColor *= texture(u_textureSamplers[10], v_texCoordOut * v_TilingFactorOut); break;
        case 11: texColor *= texture(u_textureSamplers[11], v_texCoordOut * v_TilingFactorOut); break;
        case 12: texColor *= texture(u_textureSamplers[12], v_texCoordOut * v_TilingFactorOut); break;
        case 13: texColor *= texture(u_textureSamplers[13], v_texCoordOut * v_TilingFactorOut); break;
        case 14: texColor *= texture(u_textureSamplers[14], v_texCoordOut * v_TilingFactorOut); break;
        case 15: texColor *= texture(u_textureSamplers[15], v_texCoordOut * v_TilingFactorOut); break;
        case 16: texColor *= texture(u_textureSamplers[16], v_texCoordOut * v_TilingFactorOut); break;
        case 17: texColor *= texture(u_textureSamplers[17], v_texCoordOut * v_TilingFactorOut); break;
        case 18: texColor *= texture(u_textureSamplers[18], v_texCoordOut * v_TilingFactorOut); break;
        case 19: texColor *= texture(u_textureSamplers[19], v_texCoordOut * v_TilingFactorOut); break;
        case 20: texColor *= texture(u_textureSamplers[20], v_texCoordOut * v_TilingFactorOut); break;
        case 21: texColor *= texture(u_textureSamplers[21], v_texCoordOut * v_TilingFactorOut); break;
        case 22: texColor *= texture(u_textureSamplers[22], v_texCoordOut * v_TilingFactorOut); break;
        case 23: texColor *= texture(u_textureSamplers[23], v_texCoordOut * v_TilingFactorOut); break;
        case 24: texColor *= texture(u_textureSamplers[24], v_texCoordOut * v_TilingFactorOut); break;
        case 25: texColor *= texture(u_textureSamplers[25], v_texCoordOut * v_TilingFactorOut); break;
        case 26: texColor *= texture(u_textureSamplers[26], v_texCoordOut * v_TilingFactorOut); break;
        case 27: texColor *= texture(u_textureSamplers[27], v_texCoordOut * v_TilingFactorOut); break;
        case 28: texColor *= texture(u_textureSamplers[28], v_texCoordOut * v_TilingFactorOut); break;
        case 29: texColor *= texture(u_textureSamplers[29], v_texCoordOut * v_TilingFactorOut); break;
        case 30: texColor *= texture(u_textureSamplers[30], v_texCoordOut * v_TilingFactorOut); break;
        case 31: texColor *= texture(u_textureSamplers[31], v_texCoordOut * v_TilingFactorOut); break;
    }

    v_color = texColor;
    objectID = v_EntityIDOut;
}
