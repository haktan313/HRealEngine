
#type vertex
#version 450 core

layout(location = 0) in vec3 v_WorldPosition;
layout(location = 1) in vec3 v_LocalPosition;
layout(location = 2) in vec4 v_Color;
layout(location = 3) in float v_Thickness;
layout(location = 4) in float v_Fade;
layout(location = 5) in int v_EntityID;

layout(std140, binding = 0) uniform u_Camera
{
    mat4 u_ViewProjectionMatrix;
};

struct VertexOut
{
    vec3 localPosition;
    vec4 color;
    float thickness;
    float fade;
};

layout (location = 0) out VertexOut Output;
layout (location = 4) out flat int v_EntityIDOut;

void main()
{
    Output.localPosition = v_LocalPosition;
    Output.color = v_Color;
    Output.thickness = v_Thickness;
    Output.fade = v_Fade;

    v_EntityIDOut = v_EntityID;

    gl_Position = u_ViewProjectionMatrix * vec4(v_WorldPosition, 1.0f);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 v_color;
layout(location = 1) out int objectID;

struct VertexOut
{
    vec3 localPosition;
    vec4 color;
    float thickness;
    float fade;
};

layout (location = 0) in VertexOut Input;
layout (location = 4) in flat int v_EntityIDOut;

void main()
{
    float dist = 1.0 - length(Input.localPosition);
    float circle = smoothstep(0.0, Input.fade, dist);
    circle *= smoothstep(Input.thickness + Input.fade, Input.thickness, dist);

    if(circle < 0.0)
        discard;

    v_color = Input.color;
    v_color.a *= circle;
    
    objectID = v_EntityIDOut;
}