
//OpenGLRendererAPI.h
#pragma once
#include "HRealEngine/Renderer/RendererAPI.h"

namespace HRealEngine
{
    class OpenGLRendererAPI : public RendererAPI
    {
    public:
        virtual void Init() override;
        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
        
        virtual void SetClearColor(const glm::vec4& color) override;
        virtual void Clear() override;

        virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t IndexCount = 0) override;
    };
    
}
