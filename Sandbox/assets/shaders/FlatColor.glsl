
//FlatColor.glsl
#type vertex
#version 330 core
layout(location = 0) in vec3 v_Position;

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_Transform;

void main()
{
	gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(v_Position, 1.0f);
}

#type fragment
#version 330 core
layout(location = 0) out vec4 v_color;

uniform vec4 u_Color;

void main()
{
    v_color = u_Color;
}