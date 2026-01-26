#pragma once
#include "HRealEngine/Camera/EditorCamera.h"
#include "HRealEngine/Core/Components.h"
#include "HRealEngine/Core/MeshLoader.h"

namespace HRealEngine
{
    class Renderer3D
    {
    public:
        static void Init();
        static void Shutdown();

        static void DrawSelectionBounds(const glm::mat4& transform, const glm::vec3& min, const glm::vec3& max, const glm::vec4& color);

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
        
        static Ref<MeshGPU> BuildStaticMeshGPU(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices,const Ref<Shader>& shader, glm::vec3& inMin, glm::vec3& inMax);
        static void DrawMesh(const glm::mat4& transform, MeshRendererComponent& meshRenderer, int entityID = -1);
        
        static void BeginShadowPass(const glm::vec3& lightDirection, const glm::vec3& focusPosition);
        static void EndShadowPass();
        static void DrawMeshShadow(const glm::mat4& transform, MeshRendererComponent& meshRenderer);
        
        // Renderer3D.h
        static void DrawMeshPointShadow(const glm::mat4& transform, MeshRendererComponent& meshRenderer);
        static void BeginPointShadowAtlas();
        static void BeginPointShadowCaster(uint32_t casterIndex, int lightIndex, const glm::vec3& lightPosition, float farPlane);

        static void EndPointShadowAtlas();
        static void EndPointShadowCaster();
    };
}
