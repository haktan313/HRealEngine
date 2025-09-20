
//RenderCommand.h
#pragma once
#include "RendererAPI.h"

namespace HRealEngine
{
    class RenderCommand
    {
    public:
        inline static void Init()
        {
            rendererAPI->Init();
        }
        inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
        {
            rendererAPI->SetViewport(x, y, width, height);
        }
        inline static void SetClearColor(const glm::vec4& color)
        {
            rendererAPI->SetClearColor(color);
        }
        inline static void Clear()
        {
            rendererAPI->Clear();
        }
        inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t IndexCount = 0)
        {
            rendererAPI->DrawIndexed(vertexArray, IndexCount);
        }
        inline static void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
        {
            rendererAPI->DrawLines(vertexArray, vertexCount);
        }
        inline static void SetLineWidth(float width)
        {
            rendererAPI->SetLineWidth(width);
        }
        
    private:
        static RendererAPI* rendererAPI;
    };
}
