
//Buffers.h
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "HRealEngine/Core/Core.h"

namespace HRealEngine
{
    enum class ShaderDataType
    {
        None = 0,
        Float, Float2, Float3, Float4,
        Mat3, Mat4,
        Int, Int2, Int3, Int4,
        Bool
    };
    static uint32_t ShaderDataTypeSize(ShaderDataType type)
    {
        switch (type)
        {
            case ShaderDataType::None: return 0;
            case ShaderDataType::Float: return 4;
            case ShaderDataType::Float2: return 4 * 2;
            case ShaderDataType::Float3: return 4 * 3;
            case ShaderDataType::Float4: return 4 * 4;
            case ShaderDataType::Mat3: return 4 * 3 * 3;
            case ShaderDataType::Mat4: return 4 * 4 * 4;
            case ShaderDataType::Int: return 4;
            case ShaderDataType::Int2: return 4 * 2;
            case ShaderDataType::Int3: return 4 * 3;
            case ShaderDataType::Int4: return 4 * 4;
            case ShaderDataType::Bool: return sizeof(bool);
        }
        HREALENGINE_CORE_DEBUGBREAK(false, "Unknown ShaderDataType!");
        return 0; 
    }
    struct BufferElement
    {
        std::string Name;
        uint32_t Size;
        uint32_t Offset;
        ShaderDataType Type;
        bool Normalized;

        BufferElement(){}

        uint32_t GetComponentCount() const
        {
            switch (Type)
            {
                case ShaderDataType::Float: return 1;
                case ShaderDataType::Float2: return 2;
                case ShaderDataType::Float3: return 3;
                case ShaderDataType::Float4: return 4;
                case ShaderDataType::Mat3: return 3 * 3;
                case ShaderDataType::Mat4: return 4 * 4;
                case ShaderDataType::Int: return 1;
                case ShaderDataType::Int2: return 2;
                case ShaderDataType::Int3: return 3;
                case ShaderDataType::Int4: return 4;
                case ShaderDataType::Bool: return 1;
            }
            HREALENGINE_CORE_DEBUGBREAK(false, "Unknown ShaderDataType!");
            return 0; 
        }

        BufferElement(const std::string& name, ShaderDataType type, bool bNormalized) : Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(bNormalized) {}
    };
    class BufferLayout
    {
    public:
        BufferLayout(){}
        BufferLayout(const std::initializer_list<BufferElement>& elements) : elementsRef(elements)
        {
            CalculateOffsetsAndStride();
        }
        std::vector<BufferElement>::iterator begin() { return elementsRef.begin(); }
        std::vector<BufferElement>::iterator end() { return elementsRef.end(); }
        std::vector<BufferElement>::const_iterator begin() const { return elementsRef.begin(); }
        std::vector<BufferElement>::const_iterator end() const { return elementsRef.end(); }

        inline uint32_t GetStride() const { return stride; }
        inline const std::vector<BufferElement>& GetElements() const { return elementsRef; }
    private:
        void CalculateOffsetsAndStride()
        {
            uint32_t offset = 0;
            stride = 0;
            for (auto& element : elementsRef)
            {
                element.Offset = offset;
                offset += element.Size;
                stride += element.Size;
            }
        }
        std::vector<BufferElement> elementsRef;
        uint32_t stride = 0;
    };
    class VertexBuffer
    {
    public:
        virtual ~VertexBuffer() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual const BufferLayout& GetLayout() const = 0;
        virtual void SetLayout(const BufferLayout& layout) = 0;

        virtual void SetData(const void* data, uint32_t size) = 0;

        static Ref<VertexBuffer> Create(float* vertices, uint32_t size);
        static Ref<VertexBuffer> Create(uint32_t size);
    };

    class IndexBuffer
    {
    public:
        virtual ~IndexBuffer() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
        virtual uint32_t GetCount() const = 0;

        static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);
    };
}
