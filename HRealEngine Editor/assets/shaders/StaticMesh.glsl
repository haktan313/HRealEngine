#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 v_Normal;
out vec2 v_TexCoord;

void main()
{
    v_Normal = mat3(transpose(inverse(u_Transform))) * a_Normal;
    v_TexCoord = a_TexCoord;
    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

in vec3 v_Normal;
in vec2 v_TexCoord;

uniform int u_DebugView; 
// 0 = normal render
// 1 = UV debug
// 2 = Normal debug


uniform vec4 u_Color = vec4(1.0);
uniform sampler2D u_Albedo;
uniform int u_HasAlbedo = 0;

void main()
{
    if (u_DebugView == 1)
    {
        o_Color = vec4(fract(v_TexCoord), 0.0, 1.0);
        return;
    }
    if (u_DebugView == 2)
    {
        vec3 n = normalize(v_Normal) * 0.5 + 0.5;
        o_Color = vec4(n, 1.0);
        return;
    }


    if (u_HasAlbedo == 1)
    {
        vec4 albedo = texture(u_Albedo, v_TexCoord);
        o_Color = albedo * u_Color;
    }
    else
        o_Color = u_Color;
   
}

