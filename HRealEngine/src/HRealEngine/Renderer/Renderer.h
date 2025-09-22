

#pragma once
#include "RendererAPI.h"
#include "VertexArray.h"
#include "Shader.h"
#include "HRealEngine/Camera/OrthCamera.h"

namespace HRealEngine
{
    class Renderer
    {
    public:
        static void Init();
        static void Shutdown();
        
        static void OnWindowResize(uint32_t width, uint32_t height);
        
        static void BeginScene(OrthCamera& orthCameraRef);
        static void EndScene();
        
        static void Submit(const Ref<VertexArray>& vertexArray, const Ref<Shader>& shaderRef, const glm::mat4& transform = glm::mat4(1.0f));
        
        static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
    private:
        struct SceneData
        {
            glm::mat4 viewProjectionMatrix;
        };
        static Scope<SceneData> s_SceneData;
    };
}
