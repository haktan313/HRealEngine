#type vertex
#version 450 core
layout(location=0) in vec3 a_Position;

uniform mat4 u_Model;

out vec3 v_WorldPos;

void main()
{
    vec4 world = u_Model * vec4(a_Position, 1.0);
    v_WorldPos = world.xyz;
    gl_Position = world;
}

#type geometry
#version 450 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

in vec3 v_WorldPos[];

out vec3 g_WorldPos;

uniform mat4 u_ShadowMatrices[6];

void main()
{
    for (int face = 0; face < 6; face++)
    {
        gl_Layer = face;

        for (int i = 0; i < 3; i++)
        {
            g_WorldPos = v_WorldPos[i];
            gl_Position = u_ShadowMatrices[face] * vec4(g_WorldPos, 1.0);
            EmitVertex();
        }
        EndPrimitive();
    }
}

#type fragment
#version 450 core
in vec3 g_WorldPos;

uniform vec3  u_LightPos;
uniform float u_FarPlane;

void main()
{
    float dist = length(g_WorldPos - u_LightPos);
    gl_FragDepth = dist / u_FarPlane;
}


