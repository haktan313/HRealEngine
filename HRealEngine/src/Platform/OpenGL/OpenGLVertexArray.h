
//OpenGLVertexArray.h
#pragma once
#include <memory>
#include <vector>

#include "HRealEngine/Renderer/VertexArray.h"

namespace HRealEngine
{
    class OpenGLVertexArray : public VertexArray
    {
    public:
        OpenGLVertexArray();
        virtual ~OpenGLVertexArray();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
        virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;

        virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override { return vertexBuffers;}
        virtual const Ref<IndexBuffer>& GetIndexBuffer() const override{ return indexBufferRef;}
    private:
        std::vector<Ref<VertexBuffer>> vertexBuffers;
        Ref<IndexBuffer> indexBufferRef;
        uint32_t rendererID;
    };
}
