
#type vertex
#version 450 core

layout(location = 0) in vec3 v_Position;
layout(location = 5) in int v_EntityID;
layout(standard140, binding = 0) uniform u_Camera
{
    mat4 u_ViewProjectionMatrix;
};
struct VertexOut
{
    vec4 color;
};

layout (location = 0) out VertexOut Output;
layout (location = 4) out flat int v_EntityIDOut;

void main()
{
    Output.color = v_Color;
    v_EntityIDOut = v_EntityID;
    gl_Position = u_ViewProjectionMatrix * vec4(v_Position, 1.0f);
}




#type fragment
#version 450 core

layout(location = 0) out vec4 v_color;
layout(location = 1) out int objectID;


void main()
{
    v_color = Input.color;
    objectID = v_EntityIDOut;
}
