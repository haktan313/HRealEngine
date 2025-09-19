
//Texture.glsl
#type vertex
#version 330 core
layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec4 v_Color;
layout(location = 2) in vec2 v_texCoordIn;
layout(location = 3) in float v_TexIndex;
layout(location = 4) in float v_TilingFactor;

uniform mat4 u_ViewProjectionMatrix;
 
out vec4 v_ColorOut;
out vec2 v_texCoordOut;
out float v_TexIndexOut;
out float v_TilingFactorOut;
void main()
{
    v_ColorOut = v_Color;
	v_texCoordOut = v_texCoordIn;
    v_TexIndexOut = v_TexIndex;
    v_TilingFactorOut = v_TilingFactor;
	gl_Position = u_ViewProjectionMatrix * vec4(v_Position, 1.0f);
}

#type fragment
#version 330 core
layout(location = 0) out vec4 v_color;

in vec4 v_ColorOut;
in vec2 v_texCoordOut;
in float v_TexIndexOut;
in float v_TilingFactorOut;

uniform sampler2D u_textureSamplers[32];

void main()
{
	v_color = texture(u_textureSamplers[int(v_TexIndexOut)], v_texCoordOut * v_TilingFactorOut) * v_ColorOut;
}