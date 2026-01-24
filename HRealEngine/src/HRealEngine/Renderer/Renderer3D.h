#pragma once
#include "HRealEngine/Camera/EditorCamera.h"
#include "HRealEngine/Core/Components.h"
#include "HRealEngine/Core/ObjLoader.h"

namespace HRealEngine
{
    class Renderer3D
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(const Camera& camera, const glm::mat4& transform);
        static void BeginScene(EditorCamera& camera);
        static void EndScene();
        static void StartBatch();
        static void Flush();

        struct LightGPU
        {
            int Type;
            glm::vec3 Position;
            glm::vec3 Direction;
            glm::vec3 Color;
            float Intensity;
            float Radius;
            int CastShadows;
        };

        static void SetLights(const std::vector<LightGPU>& lights);
        static void SetViewPosition(const glm::vec3& pos);
        
        static Ref<MeshGPU> BuildStaticMeshGPU(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices,const Ref<Shader>& shader);
        static void DrawMesh(const glm::mat4& transform, MeshRendererComponent& meshRenderer, int entityID = -1);


        static void BeginShadowPass(const glm::vec3& lightDirection, const glm::vec3& focusPosition);
        static void EndShadowPass();
        static void DrawMeshShadow(const glm::mat4& transform, MeshRendererComponent& meshRenderer);

        static bool HasShadowMap();
        static uint32_t GetShadowMapRendererID();
        static const glm::mat4& GetLightSpaceMatrix();


        // Renderer3D.h
        static void BeginPointShadowPass(const glm::vec3& lightPosition, float farPlane);
        static void EndPointShadowPass();
        static void DrawMeshPointShadow(const glm::mat4& transform, MeshRendererComponent& meshRenderer);

        static bool HasPointShadowMap();
        static uint32_t GetPointShadowMapRendererID();
        static const glm::vec3& GetPointShadowLightPos();
        static float GetPointShadowFarPlane();

        static void BeginPointShadowAtlas();
        static void BeginPointShadowCaster(uint32_t casterIndex, int lightIndex, const glm::vec3& lightPosition, float farPlane);
        static void EndPointShadowCaster();
        static void EndPointShadowAtlas();
    };
}
