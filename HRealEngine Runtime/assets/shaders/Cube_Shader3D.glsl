
#type vertex
#version 450 core

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec3 v_Normal;   
layout(location = 2) in vec4 v_Color;
layout(location = 3) in vec2 v_texCoordIn;
layout(location = 4) in float v_TexIndex;
layout(location = 5) in float v_TilingFactor;
layout(location = 6) in int v_EntityID;

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
    vec3 worldPos;
    vec3 normal;
};

layout (location = 0) out VertexOut Output;
layout (location = 6) out flat int v_EntityIDOut;

void main()
{
    Output.color = v_Color;
    Output.texCoord = v_texCoordIn;
    Output.texIndex = v_TexIndex;
    Output.tilingFactor = v_TilingFactor;

    Output.worldPos = v_Position;
    Output.normal = normalize(v_Normal);

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
    vec3 worldPos;
    vec3 normal;
};

uniform int u_DebugView; 
// 0 = normal render
// 1 = UV debug

layout (location = 0) in VertexOut Input;
layout (location = 6) in flat int v_EntityIDOut;

layout (binding = 0) uniform sampler2D u_textureSamplers[30];

#define MAX_LIGHTS 16
struct Light
{
    int Type; // 0 dir, 1 point, 2 spot
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
uniform float u_Shininess;


uniform int u_HasShadowMap = 0;
uniform sampler2D u_ShadowMap;
uniform mat4 u_LightSpaceMatrix;
uniform float u_ShadowBias;

float ComputeShadow(vec3 worldPos, vec3 normal, vec3 lightDir)
{
    vec4 lightClip = u_LightSpaceMatrix * vec4(worldPos, 1.0);
    vec3 proj = lightClip.xyz / lightClip.w;//make 3D, w is perspective
    proj = proj * 0.5 + 0.5;//to 0-1 range from -1 to 1 // (x - (-1)) / (1 - (-1)) * (1 - 0) + 0 y = (x - minA) / (maxA - minA) * (maxB - minB) + minB

    if (proj.z > 1.0)//proj.z is depth in light space
         return 0.0;
        
    float currentDepth = proj.z;
    float bias = max(u_ShadowBias * (1.0 - dot(normal, lightDir)), u_ShadowBias * 0.1);

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
uniform samplerCubeArray u_PointShadowMaps;
uniform vec3  u_PointShadowLightPos[MAX_LIGHTS];
uniform float u_PointShadowFarPlane[MAX_LIGHTS];
uniform int   u_PointShadowIndex[MAX_LIGHTS];

float ComputePointShadow(vec3 worldPos, int lightIndex)
{
    int shadowIdx = u_PointShadowIndex[lightIndex];
    if (shadowIdx < 0) 
        return 0.0;

    vec3 fragToLight = worldPos - u_PointShadowLightPos[lightIndex];
    float currentDepth = length(fragToLight);

    float closestDepth = texture(u_PointShadowMaps, vec4(fragToLight, float(shadowIdx))).r
                         * u_PointShadowFarPlane[lightIndex];

    float bias = 0.05;
    return (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
}


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
        //case 30: texColor *= texture(u_textureSamplers[30], Input.texCoord * Input.tilingFactor); break;
        //case 31: texColor *= texture(u_textureSamplers[31], Input.texCoord * Input.tilingFactor); break;
    }

    if(texColor.a < 0.1)
        discard;

    objectID = v_EntityIDOut;

    vec3 normal = normalize(Input.normal);
    if (!gl_FrontFacing)
		 normal = -normal;

    if (u_DebugView == 1)
    {
        v_color = vec4(fract(Input.texCoord), 0.0, 1.0);
        return;
    }
    if (u_DebugView == 2)
    {
        v_color = vec4(normal * 0.5 + 0.5, 1.0);
        return;
    }

    vec3 baseColor = texColor.rgb;
    vec3 viewDirection = normalize(u_ViewPos - Input.worldPos);

    float shininess = max(u_Shininess, 1.0);
    vec3 lit = 0.05 * baseColor;

    for (int i = 0; i < u_LightCount; i++)
    {
        Light light = u_Lights[i];

        vec3 lightDirection;
        float atten = 1.0;

        if (light.Type == 0)
			 lightDirection = normalize(-light.Direction);
        else
        {
            vec3 toLight = light.Position - Input.worldPos;
            float dist = length(toLight);
            lightDirection = (dist > 0.0001) ? (toLight / dist) : vec3(0, 1, 0);

            if (light.Radius > 0.0)
            {
                float t = clamp(1.0 - dist / light.Radius, 0.0, 1.0);
                atten = t * t;
            }
        }

        float NdotL = max(dot(normal, lightDirection), 0.0);
        vec3 H = normalize(lightDirection + viewDirection);
        float specPow = pow(max(dot(normal, H), 0.0), shininess);

        vec3 diffuse  = baseColor * NdotL;
        vec3 specular = vec3(specPow);

        vec3 lightColor = light.Color * light.Intensity;

        float shadow = 0.0;
        if (u_HasShadowMap == 1 && light.Type == 0 && light.CastShadows == 1)
            shadow = ComputeShadow(Input.worldPos, normal, lightDirection);
        if (u_HasPointShadowMap == 1 && light.Type == 1 && light.CastShadows == 1)
        {
            shadow = max(shadow, ComputePointShadow(Input.worldPos, i));
        }

		lit += (diffuse + specular) * lightColor * atten * (1.0 - shadow);
    }

    v_color = vec4(lit, texColor.a);
}
