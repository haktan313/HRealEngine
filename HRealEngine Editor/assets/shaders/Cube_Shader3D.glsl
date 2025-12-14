
#type vertex
#version 450 core

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec4 v_Color;
layout(location = 2) in vec2 v_texCoordIn;
layout(location = 3) in float v_TexIndex;
layout(location = 4) in float v_TilingFactor;
layout(location = 5) in int v_EntityID;

layout(std140, binding = 0) uniform u_Camera
{
    mat4 u_ViewProjectionMatrix;
};
struct VertexOut
{
    vec4 color;
    vec2 texCoord;
    float texIndex;
    float tilingFactor;
};

layout (location = 0) out VertexOut Output;
layout (location = 4) out flat int v_EntityIDOut;

void main()
{
    Output.color = v_Color;
    Output.texCoord = v_texCoordIn;
    Output.texIndex = v_TexIndex;
    Output.tilingFactor = v_TilingFactor;

    v_EntityIDOut = v_EntityID;
    gl_Position = u_ViewProjectionMatrix * vec4(v_Position, 1.0f);
}




#type fragment
#version 450 core

layout(location = 0) out vec4 v_color;
layout(location = 1) out int objectID;

struct VertexOut
{
    vec4 color;
    vec2 texCoord;
    float texIndex;
    float tilingFactor;
};

layout (location = 0) in VertexOut Input;
layout (location = 4) in flat int v_EntityIDOut;

layout (binding = 0) uniform sampler2D u_textureSamplers[32];

void main()
{
    vec4 texColor = Input.color;

    switch(int(Input.texIndex))
    {
        case 0: texColor *= texture(u_textureSamplers[0], Input.texCoord * Input.tilingFactor); break;
        case 1: texColor *= texture(u_textureSamplers[1], Input.texCoord * Input.tilingFactor); break;
        case 2: texColor *= texture(u_textureSamplers[2], Input.texCoord * Input.tilingFactor); break;
        case 3: texColor *= texture(u_textureSamplers[3], Input.texCoord * Input.tilingFactor); break;
        case 4: texColor *= texture(u_textureSamplers[4], Input.texCoord * Input.tilingFactor); break;
        case 5: texColor *= texture(u_textureSamplers[5], Input.texCoord * Input.tilingFactor); break;
        case 6: texColor *= texture(u_textureSamplers[6], Input.texCoord * Input.tilingFactor); break;
        case 7: texColor *= texture(u_textureSamplers[7], Input.texCoord * Input.tilingFactor); break;
        case 8: texColor *= texture(u_textureSamplers[8], Input.texCoord * Input.tilingFactor); break;
        case 9: texColor *= texture(u_textureSamplers[9], Input.texCoord * Input.tilingFactor); break;
        case 10: texColor *= texture(u_textureSamplers[10], Input.texCoord * Input.tilingFactor); break;
        case 11: texColor *= texture(u_textureSamplers[11], Input.texCoord * Input.tilingFactor); break;
        case 12: texColor *= texture(u_textureSamplers[12], Input.texCoord * Input.tilingFactor); break;
        case 13: texColor *= texture(u_textureSamplers[13], Input.texCoord * Input.tilingFactor); break;
        case 14: texColor *= texture(u_textureSamplers[14], Input.texCoord * Input.tilingFactor); break;
        case 15: texColor *= texture(u_textureSamplers[15], Input.texCoord * Input.tilingFactor); break;
        case 16: texColor *= texture(u_textureSamplers[16], Input.texCoord * Input.tilingFactor); break;
        case 17: texColor *= texture(u_textureSamplers[17], Input.texCoord * Input.tilingFactor); break;
        case 18: texColor *= texture(u_textureSamplers[18], Input.texCoord * Input.tilingFactor); break;
        case 19: texColor *= texture(u_textureSamplers[19], Input.texCoord * Input.tilingFactor); break;
        case 20: texColor *= texture(u_textureSamplers[20], Input.texCoord * Input.tilingFactor); break;
        case 21: texColor *= texture(u_textureSamplers[21], Input.texCoord * Input.tilingFactor); break;
        case 22: texColor *= texture(u_textureSamplers[22], Input.texCoord * Input.tilingFactor); break;
        case 23: texColor *= texture(u_textureSamplers[23], Input.texCoord * Input.tilingFactor); break;
        case 24: texColor *= texture(u_textureSamplers[24], Input.texCoord * Input.tilingFactor); break;
        case 25: texColor *= texture(u_textureSamplers[25], Input.texCoord * Input.tilingFactor); break;
        case 26: texColor *= texture(u_textureSamplers[26], Input.texCoord * Input.tilingFactor); break;
        case 27: texColor *= texture(u_textureSamplers[27], Input.texCoord * Input.tilingFactor); break;
        case 28: texColor *= texture(u_textureSamplers[28], Input.texCoord * Input.tilingFactor); break;
        case 29: texColor *= texture(u_textureSamplers[29], Input.texCoord * Input.tilingFactor); break;
        case 30: texColor *= texture(u_textureSamplers[30], Input.texCoord * Input.tilingFactor); break;
        case 31: texColor *= texture(u_textureSamplers[31], Input.texCoord * Input.tilingFactor); break;
    }

    if(texColor.a < 0.1)
        discard;
    v_color = texColor;
    objectID = v_EntityIDOut;
}
