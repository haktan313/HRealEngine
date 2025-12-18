#include "HRpch.h"
#include "Renderer3D.h"

#include "RenderCommand.h"
#include "Shader.h"
#include "UniformBuffer.h"
#include "VertexArray.h"
#include "HRealEngine/Core/ObjLoader.h"

namespace HRealEngine
{
    struct CubeVertex
    {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        float TexIndex;
        float TilingFactor;
        
        int EntityID;
    };
    struct Renderer3DData
    {
        static const uint32_t MaxCubes = 1000;
        static const uint32_t MaxVertices = MaxCubes * 24;
        static const uint32_t MaxIndices = MaxCubes * 36;
        static const uint32_t MaxTextureSlots = 32;

        Ref<Texture2D> WhiteTexture;
        std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1; //0 = white texture
        
        Ref<VertexArray> CubeVertexArray;
        Ref<VertexBuffer> CubeVertexBuffer;
        Ref<Shader> CubeShader;

        uint32_t CubeIndexCount = 0;
        CubeVertex* CubeVertexBufferBase = nullptr;
        CubeVertex* CubeVertexBufferPtr = nullptr;
        
        struct CameraData
        {
            glm::mat4 ViewProjectionMatrix;
        };
        CameraData CameraBuffer;
        Ref<UniformBuffer> CameraUniformBuffer;

        glm::vec4 VertexPos[24];
        glm::vec2 VertexUV[24];
    };
    static Renderer3DData s_Data;
    
    void Renderer3D::Init()
    {
        s_Data.CubeVertexArray = VertexArray::Create();
        s_Data.CubeVertexBuffer = VertexBuffer::Create(sizeof(CubeVertex) * s_Data.MaxVertices);
        s_Data.CubeVertexBuffer->SetLayout({
        {"v_Position", ShaderDataType::Float3, false},
        {"v_Color", ShaderDataType::Float4, false},
        {"v_TexCoord", ShaderDataType::Float2, false},
        {"v_TexIndex", ShaderDataType::Float,  false},
        {"v_TilingFactor", ShaderDataType::Float,  false},
        {"v_EntityID", ShaderDataType::Int, false}
        });
        s_Data.CubeVertexArray->AddVertexBuffer(s_Data.CubeVertexBuffer);
        s_Data.CubeVertexBufferBase = new CubeVertex[s_Data.MaxVertices];
        
        uint32_t* cubeIndices = new uint32_t[s_Data.MaxIndices];
        uint32_t offset = 0;
        
        for (uint32_t i = 0; i < s_Data.MaxIndices; i += 36)
        {
            for(int j=0; j<6; j++) // 6 faces
            {
                cubeIndices[i + j*6 + 0] = offset + 0;
                cubeIndices[i + j*6 + 1] = offset + 1;
                cubeIndices[i + j*6 + 2] = offset + 2;
                
                cubeIndices[i + j*6 + 3] = offset + 2;
                cubeIndices[i + j*6 + 4] = offset + 3;
                cubeIndices[i + j*6 + 5] = offset + 0;
                offset += 4;
            }
        }

        Ref<IndexBuffer> cubeIndexBufferRef = IndexBuffer::Create(cubeIndices, s_Data.MaxIndices);
        s_Data.CubeVertexArray->SetIndexBuffer(cubeIndexBufferRef);
        delete[] cubeIndices;

        s_Data.WhiteTexture = Texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        int32_t samplers[s_Data.MaxTextureSlots];
        for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
            samplers[i] = i;

        s_Data.CubeShader = Shader::Create("assets/shaders/Cube_Shader3D.glsl");
        
        s_Data.TextureSlots[0] = s_Data.WhiteTexture;

        s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::CameraData), 0);
        
        // Front Face (Z = 0.5)
        s_Data.VertexPos[0] = {-0.5f, -0.5f,  0.5f, 1.0f}; s_Data.VertexUV[0] = {0, 0};
        s_Data.VertexPos[1] = { 0.5f, -0.5f,  0.5f, 1.0f}; s_Data.VertexUV[1] = {1, 0};
        s_Data.VertexPos[2] = { 0.5f,  0.5f,  0.5f, 1.0f}; s_Data.VertexUV[2] = {1, 1};
        s_Data.VertexPos[3] = {-0.5f,  0.5f,  0.5f, 1.0f}; s_Data.VertexUV[3] = {0, 1};
        // Back Face (Z = -0.5)
        s_Data.VertexPos[4] = { 0.5f, -0.5f, -0.5f, 1.0f}; s_Data.VertexUV[4] = {0, 0};
        s_Data.VertexPos[5] = {-0.5f, -0.5f, -0.5f, 1.0f}; s_Data.VertexUV[5] = {1, 0};
        s_Data.VertexPos[6] = {-0.5f,  0.5f, -0.5f, 1.0f}; s_Data.VertexUV[6] = {1, 1};
        s_Data.VertexPos[7] = { 0.5f,  0.5f, -0.5f, 1.0f}; s_Data.VertexUV[7] = {0, 1};
        // Top Face (Y = 0.5)
        s_Data.VertexPos[8]  = {-0.5f,  0.5f,  0.5f, 1.0f}; s_Data.VertexUV[8]  = {0, 0};
        s_Data.VertexPos[9]  = { 0.5f,  0.5f,  0.5f, 1.0f}; s_Data.VertexUV[9]  = {1, 0};
        s_Data.VertexPos[10] = { 0.5f,  0.5f, -0.5f, 1.0f}; s_Data.VertexUV[10] = {1, 1};
        s_Data.VertexPos[11] = {-0.5f,  0.5f, -0.5f, 1.0f}; s_Data.VertexUV[11] = {0, 1};
        // Under Face (Y = -0.5)
        s_Data.VertexPos[12] = {-0.5f, -0.5f, -0.5f, 1.0f}; s_Data.VertexUV[12] = {0, 0};
        s_Data.VertexPos[13] = { 0.5f, -0.5f, -0.5f, 1.0f}; s_Data.VertexUV[13] = {1, 0};
        s_Data.VertexPos[14] = { 0.5f, -0.5f,  0.5f, 1.0f}; s_Data.VertexUV[14] = {1, 1};
        s_Data.VertexPos[15] = {-0.5f, -0.5f,  0.5f, 1.0f}; s_Data.VertexUV[15] = {0, 1};
        // Right Face (X = 0.5)
        s_Data.VertexPos[16] = { 0.5f, -0.5f,  0.5f, 1.0f}; s_Data.VertexUV[16] = {0, 0};
        s_Data.VertexPos[17] = { 0.5f, -0.5f, -0.5f, 1.0f}; s_Data.VertexUV[17] = {1, 0};
        s_Data.VertexPos[18] = { 0.5f,  0.5f, -0.5f, 1.0f}; s_Data.VertexUV[18] = {1, 1};
        s_Data.VertexPos[19] = { 0.5f,  0.5f,  0.5f, 1.0f}; s_Data.VertexUV[19] = {0, 1};
        // Left Face (X = -0.5)
        s_Data.VertexPos[20] = {-0.5f, -0.5f, -0.5f, 1.0f}; s_Data.VertexUV[20] = {0, 0};
        s_Data.VertexPos[21] = {-0.5f, -0.5f,  0.5f, 1.0f}; s_Data.VertexUV[21] = {1, 0};
        s_Data.VertexPos[22] = {-0.5f,  0.5f,  0.5f, 1.0f}; s_Data.VertexUV[22] = {1, 1};
        s_Data.VertexPos[23] = {-0.5f,  0.5f, -0.5f, 1.0f}; s_Data.VertexUV[23] = {0, 1};
    }

    void Renderer3D::Shutdown()
    {
        delete[] s_Data.CubeVertexBufferBase;
    }

    void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        s_Data.CameraBuffer.ViewProjectionMatrix = camera.GetProjectionMatrix() * glm::inverse(transform);
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer3DData::CameraData));
        StartBatch();
    }

    void Renderer3D::BeginScene(EditorCamera& camera)
    {
        s_Data.CameraBuffer.ViewProjectionMatrix = camera.GetViewProjection();
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer3DData::CameraData));
        StartBatch();
    }

    void Renderer3D::EndScene()
    {
        Flush();
    }

    void Renderer3D::StartBatch()
    {
        s_Data.CubeIndexCount = 0;
        s_Data.CubeVertexBufferPtr = s_Data.CubeVertexBufferBase;
        s_Data.TextureSlotIndex = 1;
    }

    void Renderer3D::Flush()
    {
        if (s_Data.CubeIndexCount == 0)
            return;

        uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CubeVertexBufferPtr - (uint8_t*)s_Data.CubeVertexBufferBase);
        s_Data.CubeVertexBuffer->SetData(s_Data.CubeVertexBufferBase, dataSize);
        
        for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
            s_Data.TextureSlots[i]->Bind(i);

        s_Data.CubeShader->Bind();
        RenderCommand::DrawIndexed(s_Data.CubeVertexArray, s_Data.CubeIndexCount);
        
        // s_Data.Stats.DrawCalls++;
    }

    void Renderer3D::DrawMesh(const glm::mat4& transform, MeshRendererComponent& meshRenderer, int entityID)
    {
        if (meshRenderer.Mesh && meshRenderer.Mesh->VAO && meshRenderer.Mesh->Shader && meshRenderer.Mesh->IndexCount > 0)
        {
            meshRenderer.Mesh->Shader->Bind();
            meshRenderer.Mesh->Shader->SetInt("u_EntityID", entityID);
            meshRenderer.Mesh->Shader->SetFloat4("u_Color", meshRenderer.Color);
            meshRenderer.Mesh->Shader->SetMat4("u_ViewProjection", s_Data.CameraBuffer.ViewProjectionMatrix);
            meshRenderer.Mesh->Shader->SetMat4("u_Transform", transform);

            RenderCommand::DrawIndexed(meshRenderer.Mesh->VAO, meshRenderer.Mesh->IndexCount);
            return;
        }
        
        if (s_Data.CubeIndexCount >= s_Data.MaxIndices)
        {
            Flush();
            StartBatch();
        }
        
        float textureIndex = 0.0f;
        if(meshRenderer.Texture)
        {
            for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
            {
                if (*s_Data.TextureSlots[i] == *meshRenderer.Texture)
                {
                    textureIndex = (float)i;
                    break;
                }
            }
            if (textureIndex == 0.0f)
            {
                if (s_Data.TextureSlotIndex >= s_Data.MaxTextureSlots)
                {
                    Flush();
                    StartBatch();
                }
                textureIndex = (float)s_Data.TextureSlotIndex;
                s_Data.TextureSlots[s_Data.TextureSlotIndex] = meshRenderer.Texture;
                s_Data.TextureSlotIndex++;
            }
        }
        
        for (size_t i = 0; i < 24; i++)
        {
            s_Data.CubeVertexBufferPtr->Position = transform * s_Data.VertexPos[i];
            s_Data.CubeVertexBufferPtr->Color = meshRenderer.Color;
            s_Data.CubeVertexBufferPtr->TexCoord = s_Data.VertexUV[i];
            s_Data.CubeVertexBufferPtr->TexIndex = textureIndex;
            s_Data.CubeVertexBufferPtr->TilingFactor = meshRenderer.TilingFactor;
            s_Data.CubeVertexBufferPtr->EntityID = entityID; 
            
            s_Data.CubeVertexBufferPtr++;
        }

        s_Data.CubeIndexCount += 36;
        // s_Data.Stats.CubeCount++;
    }
}
