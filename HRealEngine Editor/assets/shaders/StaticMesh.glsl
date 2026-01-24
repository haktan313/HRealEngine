#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;//uvs

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 v_Normal;
out vec2 v_TexCoord;//uvs
out vec3 v_WorldPos;

void main()
{
    vec4 world = u_Transform * vec4(a_Position, 1.0);
    v_WorldPos = world.xyz;

    v_Normal = mat3(transpose(inverse(u_Transform))) * a_Normal;
    v_TexCoord = a_TexCoord;
    gl_Position = u_ViewProjection * world;
}



#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;// final color
layout(location = 1) out int o_EntityID;
uniform int u_EntityID;

in vec3 v_Normal;
in vec2 v_TexCoord;
in vec3 v_WorldPos;

uniform int u_DebugView; // 0 normal, 1 uv, 2 normal, 3 spec, 4 normal map raw

uniform vec4 u_Color = vec4(1.0);//material base color
uniform sampler2D u_Albedo;
uniform int u_HasAlbedo = 0;

uniform sampler2D u_Specular;
uniform int u_HasSpecular = 0;

uniform sampler2D u_Normal;
uniform int u_HasNormal = 0;

uniform float u_Shininess = 32.0;

#define MAX_LIGHTS 16
struct Light
{
    int Type;//0 dir, 1 point, 2 spot
    vec3 Position;
    vec3 Direction;
    vec3 Color;
    float Intensity;
    float Radius;
    int CastShadows;
};
uniform int u_LightCount;
uniform Light u_Lights[MAX_LIGHTS];
uniform vec3 u_ViewPos;

uniform int u_HasShadowMap = 0;
uniform sampler2D u_ShadowMap;//depth map
uniform mat4 u_LightSpaceMatrix;

float ComputeShadow(vec3 worldPos, vec3 normal, vec3 lightDir)
{
    vec4 lightClip = u_LightSpaceMatrix * vec4(worldPos, 1.0);
    vec3 proj = lightClip.xyz / lightClip.w;//make 3D, w is perspective
    proj = proj * 0.5 + 0.5;//to 0-1 range from -1 to 1 // (x - (-1)) / (1 - (-1)) * (1 - 0) + 0 y = (x - minA) / (maxA - minA) * (maxB - minB) + minB

    if (proj.z > 1.0)//proj.z is depth in light space
         return 0.0;
        
    float currentDepth = proj.z;
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);  

    float shadow = 0.0;
    vec2 texturePixelSize = 1.0 / textureSize(u_ShadowMap, 0);
    
    for(int x = -1; x <= 1; ++x)
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(u_ShadowMap, proj.xy + vec2(x, y) * texturePixelSize).r;//depth from shadow map. texture(u_ShadowMap, coords) gives uv coords from texture
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }
    shadow /= 9.0;
    return shadow;
}

uniform int u_HasPointShadowMap = 0;
uniform samplerCube u_PointShadowMap;
uniform vec3  u_PointShadowLightPos;
uniform float u_PointShadowFarPlane;
float ComputePointShadow(vec3 worldPos)
{
    vec3 fragToLight = worldPos - u_PointShadowLightPos;
    float currentDepth = length(fragToLight);

    // sample stored depth (0..1) then scale back to world distance
    float closestDepth = texture(u_PointShadowMap, fragToLight).r * u_PointShadowFarPlane;

    // basic bias (you can tune)
    float bias = 0.05;

    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;
}


void main()
{
    o_EntityID = u_EntityID;
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
    if (u_DebugView == 3)
    {
        if (u_HasSpecular == 1)
            o_Color = vec4(texture(u_Specular, v_TexCoord).rrr, 1.0);
        else
            o_Color = vec4(1.0, 0.0, 1.0, 1.0);
        return;
    }
    if (u_DebugView == 4)
    {
        if (u_HasNormal == 1)
            o_Color = vec4(texture(u_Normal, v_TexCoord).xyz, 1.0);
        else
            o_Color = vec4(1.0, 0.0, 1.0, 1.0);
        return;;
    }

    vec3 baseColor = u_Color.rgb;
    if (u_HasAlbedo == 1)
        baseColor *= texture(u_Albedo, v_TexCoord).rgb;

    vec3 normal = normalize(v_Normal);
    vec3 viewDirection = normalize(u_ViewPos - v_WorldPos);

    float shininess = max(u_Shininess, 1.0);
    vec3 lit = 0.05 * baseColor;

    for (int i = 0; i < u_LightCount; i++)
    {
        Light light = u_Lights[i];

        vec3 lightDirection;
        float atten = 1.0;

        if (light.Type == 0) // directional
            lightDirection = normalize(-light.Direction);
        else // point
        {
            vec3 toLight = light.Position - v_WorldPos;
            float dist = length(toLight);
            lightDirection = (dist > 0.0001) ? (toLight / dist) : vec3(0, 1, 0);

            if (light.Radius > 0.0)
            {
                float t = clamp(1.0 - dist / light.Radius, 0.0, 1.0);
                atten = t * t;
            }
        }

        float NdotL = max(dot(normal, lightDirection), 0.0);

        vec3 halfVector = normalize(lightDirection + viewDirection);
        float specPow = pow(max(dot(normal, halfVector), 0.0), shininess);

        float specMask = 1.0;
        if (u_HasSpecular == 1)
            specMask = texture(u_Specular, v_TexCoord).r;

        vec3 diffuse  = baseColor * NdotL;
        vec3 specular = vec3(specPow) * specMask;

        vec3 lightColor = light.Color * light.Intensity;

        float shadow = 0.0;
        if (u_HasShadowMap == 1 && light.Type == 0 && light.CastShadows == 1)
            shadow = ComputeShadow(v_WorldPos, normal, lightDirection);
        if (u_HasPointShadowMap == 1 && light.Type == 1 && light.CastShadows == 1)
        {
            float distToShadowCaster = length(light.Position - u_PointShadowLightPos);
            
            if (distToShadowCaster < 0.05) 
            {
                shadow = max(shadow, ComputePointShadow(v_WorldPos)); 
            }
        }
		lit += (diffuse + specular) * lightColor * atten * (1.0 - shadow);
    }
    o_Color = vec4(lit, u_Color.a);   
}

