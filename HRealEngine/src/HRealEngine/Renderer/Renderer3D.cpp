#include "HRpch.h"
#include "Renderer3D.h"
#include "UniformBuffer.h"

namespace HRealEngine
{
    struct MeshDataa
    {
        glm::vec3 Position;
        glm::vec3 Scale;
        
        int EntityID;
    };
    struct Renderer3DData
    {
        struct CameraData
        {
            glm::mat4 ViewProjectionMatrix;
        };
        CameraData CameraBuffer;
        Ref<UniformBuffer> CameraUniformBuffer;
    };
    static Renderer3DData s_Data;
    
    void Renderer3D::Init()
    {
    }

    void Renderer3D::Shutdown()
    {
    }

    void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        s_Data.CameraBuffer.ViewProjectionMatrix = camera.GetProjectionMatrix() * glm::inverse(transform);
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer3DData::CameraData));
    }

    void Renderer3D::BeginScene(EditorCamera& camera)
    {
        s_Data.CameraBuffer.ViewProjectionMatrix = camera.GetViewProjection();
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer3DData::CameraData));
    }

    void Renderer3D::EndScene()
    {
    }

    void Renderer3D::DrawMesh(const glm::mat4& transform, MeshRendererComponent& meshRenderer, int entityID)
    {
    }
}
