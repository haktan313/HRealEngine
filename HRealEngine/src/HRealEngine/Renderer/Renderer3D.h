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
        
        static Ref<MeshGPU> BuildStaticMeshGPU(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices,const Ref<Shader>& shader);
        static void DrawMesh(const glm::mat4& transform, MeshRendererComponent& meshRenderer, int entityID = -1);
    };
}
