

#include "HRpch.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "Renderer2D.h"

namespace HRealEngine
{
    Scope<Renderer::SceneData> Renderer::s_SceneData = CreateScope<Renderer::SceneData>();

    void Renderer::Init()
    {
        RenderCommand::Init();
        Renderer2D::Init();
    }

    void Renderer::Shutdown()
    {
        Renderer2D::Shutdown();
    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
    }

    void Renderer::BeginScene(OrthCamera& orthCameraRef)
    {
        s_SceneData->viewProjectionMatrix = orthCameraRef.GetViewProjectionMatrix();
    }

    void Renderer::EndScene()
    {
    }

    void Renderer::Submit(const Ref<VertexArray>& vertexArray, const Ref<Shader>& shaderRef, const glm::mat4& transform)
    {
        shaderRef->Bind();
        shaderRef->SetMat4("u_ViewProjection", s_SceneData->viewProjectionMatrix);
        shaderRef->SetMat4("u_Transform", transform);
        
        vertexArray->Bind();
        RenderCommand::DrawIndexed(vertexArray);
    }
}
