

#pragma once
#include "HRealEngine/Renderer/RendererAPI.h"

namespace HRealEngine
{
    class OpenGLRendererAPI : public RendererAPI
    {
    public:
        void Init() override;
        void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
        
        void SetClearColor(const glm::vec4& color) override;
        void Clear() override;

        void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t IndexCount = 0) override;
        void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t indexOffset) override;
        void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;
        void SetLineWidth(float width) override;
    };
    
}
