#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 v_Normal;

void main()
{
    // Normal transform (basitçe model matrisinden)
    v_Normal = mat3(transpose(inverse(u_Transform))) * a_Normal;
    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}


#type fragment
#version 450 core
layout(location = 0) out vec4 o_Color;

in vec3 v_Normal;

void main()
{
    vec3 n = normalize(v_Normal) * 0.5 + 0.5;
    o_Color = vec4(n, 1.0);
}

