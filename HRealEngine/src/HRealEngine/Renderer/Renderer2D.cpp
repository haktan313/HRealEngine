
//Renderer2D.cpp
#include "Renderer2D.h"
#include <array>
#include "RenderCommand.h"
#include "Shader.h"
#include "VertexArray.h"
#include "UniformBuffer.h"

#include <glm/gtc/type_ptr.hpp>
#include "glm/ext/matrix_transform.hpp"

namespace HRealEngine
{
    struct QuadVertex
    {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        float TexIndex;
        float TilingFactor;

        int EntityID; //for editor
    };

    struct CircleVertex
    {
        glm::vec3 WorldPosition;
        glm::vec3 LocalPosition;
        glm::vec4 Color;
        float Thickness;
        float Fade;

        int EntityID; //for editor
    };
    
    struct Renderer2DData
    {
        static const uint32_t MaxQuads = 20000;
        static const uint32_t MaxVertices = MaxQuads * 4;
        static const uint32_t MaxIndices = MaxQuads * 6;
        static const uint32_t MaxTextureSlots = 32; 
        
        Ref<VertexArray> QuadVertexArray;
        Ref<VertexBuffer> QuadVertexBuffer;
        Ref<Shader> QuadShader;

        Ref<VertexArray> CircleVertexArray;
        Ref<VertexBuffer> CircleVertexBuffer;
        Ref<Shader> CircleShader;

        Ref<Texture2D> WhiteTexture;

        uint32_t QuadIndexCount = 0;
        QuadVertex* QuadVertexBufferBase = nullptr;
        QuadVertex* QuadVertexBufferPtr = nullptr;
        
        uint32_t CircleIndexCount = 0;
        CircleVertex* CircleVertexBufferBase = nullptr;
        CircleVertex* CircleVertexBufferPtr = nullptr;

        std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1; //0 = white texture

        glm::vec4 QuadVertexPositions[4];
        
        Renderer2D::Statistics stats;

        struct CameraData
        {
            glm::mat4 ViewProjectionMatrix;
        };
        CameraData CameraBuffer;
        Ref<UniformBuffer> CameraUniformBuffer;
    };
    static Renderer2DData s_Data;
    
    void Renderer2D::Init()
    {
        s_Data.QuadVertexArray = VertexArray::Create();
        s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
        
        s_Data.QuadVertexBuffer->SetLayout({
            {"v_Position", ShaderDataType::Float3, false},
            {"v_Color", ShaderDataType::Float4, false},
            {"v_TexCoord", ShaderDataType::Float2, false},
            {"v_TexIndex", ShaderDataType::Float, false},
            {"v_TilingFactor", ShaderDataType::Float, false},
            {"v_EntityID", ShaderDataType::Int, false}
        });
        s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

        s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];
        
        uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];

        for (uint32_t i = 0, offset = 0; i < s_Data.MaxIndices; i += 6, offset += 4)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;
        }
        Ref<IndexBuffer> squareIndexBufferRef = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
        s_Data.QuadVertexArray->SetIndexBuffer(squareIndexBufferRef);
        delete[] quadIndices;

        
        s_Data.CircleVertexArray = VertexArray::Create();
        s_Data.CircleVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(CircleVertex));
        
        s_Data.CircleVertexBuffer->SetLayout({
            {"v_WorldPosition", ShaderDataType::Float3, false},
            {"v_LocalPosition", ShaderDataType::Float3, false},
            {"v_Color", ShaderDataType::Float4, false},
            {"v_Thickness", ShaderDataType::Float, false},
            {"v_Fade", ShaderDataType::Float, false},
            {"v_EntityID", ShaderDataType::Int, false}
        });
        s_Data.CircleVertexArray->AddVertexBuffer(s_Data.CircleVertexBuffer);
        s_Data.CircleVertexArray->SetIndexBuffer(squareIndexBufferRef);
        s_Data.CircleVertexBufferBase = new CircleVertex[s_Data.MaxVertices];

        
        s_Data.WhiteTexture = Texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        int32_t samplers[s_Data.MaxTextureSlots];
        for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
            samplers[i] = i;
        
        s_Data.QuadShader = Shader::Create("assets/shaders/Quad_Texture2D.glsl");
        s_Data.CircleShader = Shader::Create("assets/shaders/Circle_Texture2D.glsl");
        /*s_Data.QuadShader->Bind();
        s_Data.QuadShader->SetIntArray("u_textureSamplers", samplers, s_Data.MaxTextureSlots);*/

        s_Data.TextureSlots[0] = s_Data.WhiteTexture;

        s_Data.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[3] = {-0.5f,  0.5f, 0.0f, 1.0f};

        s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);
    }

    void Renderer2D::Shutdown()
    {
    } 

    void Renderer2D::BeginScene(const OrthCamera& camera)
    {
        s_Data.CameraBuffer.ViewProjectionMatrix = camera.GetViewProjectionMatrix();
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));

        StartBatch();
    }

    void Renderer2D::BeginScene(const EditorCamera& camera)
    {
        s_Data.CameraBuffer.ViewProjectionMatrix = camera.GetViewProjection();
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));

        StartBatch();
    }

    void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        s_Data.CameraBuffer.ViewProjectionMatrix = camera.GetProjectionMatrix() * glm::inverse(transform);
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));

        StartBatch();
    }

    void Renderer2D::StartBatch()
    {
        s_Data.QuadIndexCount = 0;
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

        s_Data.CircleIndexCount = 0;
        s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;

        s_Data.TextureSlotIndex = 1;
    }

    void Renderer2D::EndScene()
    {
        uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
        s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);
        Flush();
    }

    void Renderer2D::Flush()
    {
        /*if (s_Data.QuadIndexCount == 0)
            return;*/
        if (s_Data.QuadIndexCount)
        {
            uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
            s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);
            for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
                s_Data.TextureSlots[i]->Bind(i);
            s_Data.QuadShader->Bind();
            RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
            s_Data.stats.DrawCalls++;
        }
        if (s_Data.CircleIndexCount)
        {
            uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CircleVertexBufferPtr - (uint8_t*)s_Data.CircleVertexBufferBase);
            s_Data.CircleVertexBuffer->SetData(s_Data.CircleVertexBufferBase, dataSize);
            
            s_Data.CircleShader->Bind();
            RenderCommand::DrawIndexed(s_Data.CircleVertexArray, s_Data.CircleIndexCount);
            s_Data.stats.DrawCalls++;
        }
    }

    void Renderer2D::FlushAndReset()
    {
        EndScene();
        StartBatch();
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        DrawQuad({position.x, position.y, 0.0f}, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        DrawQuad(transform,color);
    }
 
    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        DrawQuad({position.x, position.y, 0.0f}, size, texture, tilingFactor, tintColor);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        DrawQuad(transform, texture, tilingFactor, tintColor);
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& texture,
        float tilingFactor, const glm::vec4& tintColor)
    {
        DrawQuad({position.x, position.y, 0.0f}, size, texture, tilingFactor, tintColor);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& texture,
        float tilingFactor, const glm::vec4& tintColor)
    {
        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
        const glm::vec2* texCoords = texture->GetTexCoords();
        const Ref<Texture2D> textureRef = texture->GetTexture();
        
        if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
            FlushAndReset();
        
        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
        {
            if (*s_Data.TextureSlots[i].get() == *textureRef.get())
            {
                textureIndex = (float)i;
                break;
            }
        }
        if (textureIndex == 0.f)
        {
            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = textureRef;
            s_Data.TextureSlotIndex++;
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = color;
            s_Data.QuadVertexBufferPtr->TexCoord = texCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.stats.QuadCount++;
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID)
    {
        constexpr size_t quadVertexCount = 4;
        const float textureIndex = 0.0f; //White Texture
        constexpr glm::vec2 texCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
        constexpr float tilingFactor = 1.0f;
        
        if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
            FlushAndReset();

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = color;
            s_Data.QuadVertexBufferPtr->TexCoord = texCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex; //White Texture
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->EntityID = entityID;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.stats.QuadCount++;
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor,
        const glm::vec4& tintColor, int entityID)
    {
        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
        constexpr glm::vec2 texCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
        
        if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
            FlushAndReset();
        
        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
        {
            if (*s_Data.TextureSlots[i].get() == *texture.get())
            {
                textureIndex = (float)i;
                break;
            }
        }
        if (textureIndex == 0.f)
        {
            if (s_Data.TextureSlotIndex >= s_Data.MaxTextureSlots)
                FlushAndReset();
            
            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = color;
            s_Data.QuadVertexBufferPtr->TexCoord = texCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->EntityID = entityID;
            s_Data.QuadVertexBufferPtr++;
        }
        
        s_Data.QuadIndexCount += 6;
        s_Data.stats.QuadCount++;
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation,
                                     const Ref<SubTexture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, texture, tilingFactor, tintColor);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation,
        const Ref<SubTexture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        constexpr size_t quadVertexCount = 4;
        const Ref<Texture2D> textureRef = texture->GetTexture();
        const glm::vec2* texCoords = texture->GetTexCoords();
        if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
            FlushAndReset();
        
        constexpr glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
        {
            if (*s_Data.TextureSlots[i].get() == *textureRef.get())
            {
                textureIndex = (float)i;
                break;
            }
        }
        if (textureIndex == 0.f)
        {
            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = textureRef;
            s_Data.TextureSlotIndex++;
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f),
            rotation, {0.0f, 0.0f, 1.0f}) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = color;
            s_Data.QuadVertexBufferPtr->TexCoord = texCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.stats.QuadCount++;
    }

    void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness, float fade, int entityID)
    {
        for (size_t i = 0; i < 4; i++)
        {
            s_Data.CircleVertexBufferPtr->WorldPosition = transform * s_Data.QuadVertexPositions[i];
            s_Data.CircleVertexBufferPtr->LocalPosition = s_Data.QuadVertexPositions[i] * 2.0f;
            s_Data.CircleVertexBufferPtr->Color = color;
            s_Data.CircleVertexBufferPtr->Thickness = thickness;
            s_Data.CircleVertexBufferPtr->Fade = fade;
            s_Data.CircleVertexBufferPtr->EntityID = entityID;
            s_Data.CircleVertexBufferPtr++;
        }
        s_Data.CircleIndexCount += 6;
        s_Data.stats.QuadCount++;
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color)
    {
        DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, color);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
    {
        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec2 texCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
        if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
            FlushAndReset();
        
        const float textureIndex = 0.0f; //White Texture
        const float tilingFactor = 1.0f;
        
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f),
    rotation, {0.0f, 0.0f, 1.0f}) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = color;
            s_Data.QuadVertexBufferPtr->TexCoord = texCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex; //White Texture
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.stats.QuadCount++;
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor,  const glm::vec4& tintColor)
    {
        DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, texture, tilingFactor, tintColor);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec2 texCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
        if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
            FlushAndReset();
        
        constexpr glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
        {
            if (*s_Data.TextureSlots[i].get() == *texture.get())
            {
                textureIndex = (float)i;
                break;
            }
        }
        if (textureIndex == 0.f)
        {
            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f),
            rotation, {0.0f, 0.0f, 1.0f}) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = color;
            s_Data.QuadVertexBufferPtr->TexCoord = texCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr++;
        }
       
        s_Data.QuadIndexCount += 6;
        s_Data.stats.QuadCount++;
    }


    void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID)
    {
        if (src.Texture)
            DrawQuad(transform, src.Texture, src.TilingFactor, src.Color, entityID);
        else
            DrawQuad(transform, src.Color, entityID);
    }

    Renderer2D::Statistics Renderer2D::GetStats()
    {
        return s_Data.stats;
    }

    void Renderer2D::ResetStats()
    {
        memset(&s_Data.stats, 0, sizeof(Statistics));
    }
}
