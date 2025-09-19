
//Renderer.cpp
#include "Renderer.h"
#include "RenderCommand.h"
#include "VertexArray.h"
#include "OrthCamera.h"
#include "Renderer2D.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace HRealEngine
{
    //RendererAPI Renderer::rendererAPI = RendererAPI::OpenGL;
    Renderer::SceneData* Renderer::sceneData = new Renderer::SceneData;

    void Renderer::Init()
    {
        RenderCommand::Init();
        Renderer2D::Init();
    }

    void Renderer::OnwindowResize(uint32_t width, uint32_t height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
    }

    void Renderer::BeginScene(OrthCamera& orthCameraRef)
    {
        sceneData->viewProjectionMatrix = orthCameraRef.GetViewProjectionMatrix();
    }

    void Renderer::EndScene()
    {
    }

    void Renderer::Submit(const Ref<VertexArray>& vertexArray, const Ref<Shader>& shaderRef, const glm::mat4& transform)
    {
        shaderRef->Bind();
        std::dynamic_pointer_cast<OpenGLShader>(shaderRef)->UploadUniformMat4("viewProjectionMatrix", sceneData->viewProjectionMatrix);
        std::dynamic_pointer_cast<OpenGLShader>(shaderRef)->UploadUniformMat4("transform", transform);
        
        vertexArray->Bind();
        RenderCommand::DrawIndexed(vertexArray);
    }
}
