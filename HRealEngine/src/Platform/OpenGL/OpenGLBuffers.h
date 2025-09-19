
//OpenGLBuffers.h
#pragma once
#include <cstdint>
#include "HRealEngine/Renderer/Buffers.h"

namespace HRealEngine
{
    class OpenGLVertexBuffer : public VertexBuffer
    {
    public:
        OpenGLVertexBuffer(float* vertices, uint32_t size);
        OpenGLVertexBuffer(uint32_t size);
        virtual ~OpenGLVertexBuffer() override;

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual const BufferLayout& GetLayout() const override { return layoutRef; }
        virtual void SetLayout(const BufferLayout& layout) override { layoutRef = layout; }

        virtual void SetData(const void* data, uint32_t size) override;
    private:
        uint32_t rendererID;
        BufferLayout layoutRef;
    };

    class OpenGLIndexBuffer : public IndexBuffer
    {
    public:
        OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
        virtual ~OpenGLIndexBuffer() override;

        virtual void Bind() const override;
        virtual void Unbind() const override;
        virtual uint32_t GetCount() const override { return countRef; }
    private:
        uint32_t rendererID;
        uint32_t countRef;
    };
}
