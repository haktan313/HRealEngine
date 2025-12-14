#pragma once
#include "HRealEngine/Camera/EditorCamera.h"
#include "HRealEngine/Core/Components.h"

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

        static void DrawMesh(const glm::mat4& transform, MeshRendererComponent& meshRenderer, int entityID = -1);
    };
}
