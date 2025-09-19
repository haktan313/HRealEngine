
//Renderer.h
#pragma once
#include "HRealEngine/Core/Core.h"
#include "OrthCamera.h"
#include "RendererAPI.h"
#include "Shader.h"
#include "VertexArray.h"

namespace HRealEngine
{
    class Renderer
    {
    public:
        static void Init();
        static void OnwindowResize(uint32_t width, uint32_t height);
        
        static void BeginScene(OrthCamera& orthCameraRef);
        static void EndScene();
        
        static void Submit(const Ref<VertexArray>& vertexArray, const Ref<Shader>& shaderRef, const glm::mat4& transform = glm::mat4(1.0f));
        inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
    private:
        struct SceneData
        {
            glm::mat4 viewProjectionMatrix;
        };
        static SceneData* sceneData;
    };
}
