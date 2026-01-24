#include "HRpch.h"
#include "Renderer3D.h"

#include "Material.h"
#include "RenderCommand.h"
#include "Renderer.h"
#include "Shader.h"
#include "UniformBuffer.h"
#include "VertexArray.h"
#include "glad/glad.h"
#include "HRealEngine/Asset/AssetManager.h"
#include "HRealEngine/Core/ObjLoader.h"

namespace HRealEngine
{
    struct CubeVertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
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
        
        static const uint32_t ReservedPointShadowSlot = 30;
        static const uint32_t ReservedDirShadowSlot = 31;
        static const uint32_t MaxUserTextureSlots = 30; // 0..29 (slot 0 is white)

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
        glm::vec3 VertexNormal[24];

        std::array<Renderer3D::LightGPU, 16> Lights;
        int LightCount = 0;
        glm::vec3 ViewPos{0.0f};
        bool LightsDirty = true;

        // Shadow mapping
        uint32_t ShadowFBO = 0;
        uint32_t ShadowDepthTexture = 0;
        uint32_t ShadowMapSize = 2048;

        Ref<Shader> ShadowDepthShader;

        bool ShadowValid = false;
        glm::mat4 LightSpaceMatrix = glm::mat4(1.0f);
        glm::vec3 ShadowLightDir = glm::vec3(0.0f, -1.0f, 0.0f);

        float ShadowBias = 0.0008f;//MVP default
        int OldViewport[4] = { 0,0,0,0 };
        int OldFBO = 0;

        // Point shadow mapping
        Ref<Shader> PointShadowDepthShader;

        uint32_t PointShadowFBO = 0;
        uint32_t PointShadowDepthCubemap = 0;
        uint32_t PointShadowMapSize = 1024;
        uint32_t MaxPointShadowCasters = 8;
        uint32_t PointShadowDepthCubemapArray = 0;
        
        std::array<int, 16> PointShadowIndex{};
        std::array<glm::vec3, 16> PointShadowLightPos{};
        std::array<float, 16> PointShadowFarPlane{};

        bool PointShadowValid = false;
    };
    static Renderer3DData s_Data;

    static void CreateShadowResources()
    {
        if (s_Data.ShadowFBO != 0)
            return;

        glGenFramebuffers(1, &s_Data.ShadowFBO);

        glGenTextures(1, &s_Data.ShadowDepthTexture);
        glBindTexture(GL_TEXTURE_2D, s_Data.ShadowDepthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, (GLsizei)s_Data.ShadowMapSize, (GLsizei)s_Data.ShadowMapSize,
            0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        
        glBindFramebuffer(GL_FRAMEBUFFER, s_Data.ShadowFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, s_Data.ShadowDepthTexture, 0);

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    static void CreatePointShadowResources()
    {
        if (s_Data.PointShadowFBO != 0)
            return;

        glGenFramebuffers(1, &s_Data.PointShadowFBO);

        glGenTextures(1, &s_Data.PointShadowDepthCubemapArray);
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, s_Data.PointShadowDepthCubemapArray);
        
        const GLsizei depthLayers = (GLsizei)(6 * s_Data.MaxPointShadowCasters);

        glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT24,
            (GLsizei)s_Data.PointShadowMapSize, (GLsizei)s_Data.PointShadowMapSize, depthLayers,
            0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, s_Data.PointShadowFBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, s_Data.PointShadowDepthCubemapArray, 0);


        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
    }
    static void UploadPointShadowArrayToShader(const Ref<Shader>& shader)
    {
        if (s_Data.PointShadowValid && s_Data.PointShadowDepthCubemapArray != 0)
        {
            shader->SetInt("u_HasPointShadowMap", 1);

            const int slot = Renderer3DData::ReservedPointShadowSlot; // 30
            shader->SetInt("u_PointShadowMaps", slot);

            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, s_Data.PointShadowDepthCubemapArray);

            for (int i = 0; i < 16; i++)
            {
                shader->SetInt   ("u_PointShadowIndex[" + std::to_string(i) + "]", s_Data.PointShadowIndex[i]);
                shader->SetFloat3("u_PointShadowLightPos[" + std::to_string(i) + "]", s_Data.PointShadowLightPos[i]);
                shader->SetFloat ("u_PointShadowFarPlane[" + std::to_string(i) + "]", s_Data.PointShadowFarPlane[i]);
            }
        }
        else
        {
            shader->SetInt("u_HasPointShadowMap", 0);
            for (int i = 0; i < 16; i++)
                shader->SetInt("u_PointShadowIndex[" + std::to_string(i) + "]", -1);
        }
    }
    
    static void UploadLightsToShader(const Ref<Shader>& shader)
    {
        shader->SetFloat3("u_ViewPos", s_Data.ViewPos);
        shader->SetInt("u_LightCount", s_Data.LightCount);
        shader->SetFloat("u_Shininess", 32.0f);

        for (int i = 0; i < s_Data.LightCount; i++)
        {
            const auto& L = s_Data.Lights[i];
            std::string p = "u_Lights[" + std::to_string(i) + "].";
            shader->SetInt   (p + "Type", L.Type);
            shader->SetFloat3(p + "Position", L.Position);
            shader->SetFloat3(p + "Direction", L.Direction);
            shader->SetFloat3(p + "Color", L.Color);
            shader->SetFloat (p + "Intensity", L.Intensity);
            shader->SetFloat (p + "Radius", L.Radius);
            shader->SetInt   (p + "CastShadows", L.CastShadows);
        }
    }
    
    void Renderer3D::Init()
    {
        s_Data.CubeVertexArray = VertexArray::Create();
        s_Data.CubeVertexBuffer = VertexBuffer::Create(sizeof(CubeVertex) * s_Data.MaxVertices);
        s_Data.CubeVertexBuffer->SetLayout({
        {"v_Position", ShaderDataType::Float3, false},
        {"v_Normal", ShaderDataType::Float3, false},
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

        s_Data.WhiteTexture = Texture2D::Create(TextureSpecification()/*1, 1*/);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data.WhiteTexture->SetData(Buffer(&whiteTextureData, sizeof(uint32_t))/*&whiteTextureData, sizeof(uint32_t)*/);

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

        // Front (0..3)  +Z
        for (int i = 0; i < 4;  i++)
            s_Data.VertexNormal[i] = {0,0, 1};
        // Back (4..7)  -Z
        for (int i = 4; i < 8;  i++)
            s_Data.VertexNormal[i] = {0,0,-1};
        // Top (8..11) +Y
        for (int i = 8; i < 12; i++)
            s_Data.VertexNormal[i] = {0, 1,0};
        // Bottom (12..15)-Y
        for (int i = 12;i < 16; i++)
            s_Data.VertexNormal[i] = {0,-1,0};
        // Right (16..19)+X
        for (int i = 16;i < 20; i++)
            s_Data.VertexNormal[i] = {1,0,0};
        // Left (20..23)-X
        for (int i = 20;i < 24; i++)
            s_Data.VertexNormal[i] = {-1,0,0};

        CreateShadowResources();
        s_Data.ShadowDepthShader = Shader::Create("assets/shaders/ShadowDepth.glsl");
        CreatePointShadowResources();
        s_Data.PointShadowDepthShader = Shader::Create("assets/shaders/PointShadowDepth.glsl");

    }

    void Renderer3D::Shutdown()
    {
        delete[] s_Data.CubeVertexBufferBase;

        if (s_Data.ShadowDepthTexture)
        {
            glDeleteTextures(1, &s_Data.ShadowDepthTexture);
            s_Data.ShadowDepthTexture = 0;
        }
        if (s_Data.ShadowFBO)
        {
            glDeleteFramebuffers(1, &s_Data.ShadowFBO);
            s_Data.ShadowFBO = 0;
        }
        s_Data.ShadowDepthShader = nullptr;
        s_Data.ShadowValid = false;
        
        if (s_Data.PointShadowDepthCubemapArray)
        {
            glDeleteTextures(1, &s_Data.PointShadowDepthCubemapArray);
            s_Data.PointShadowDepthCubemapArray = 0;
        }

        if (s_Data.PointShadowFBO)
        {
            glDeleteFramebuffers(1, &s_Data.PointShadowFBO);
            s_Data.PointShadowFBO = 0;
        }
        s_Data.PointShadowDepthShader = nullptr;
        s_Data.PointShadowValid = false;

    }

    void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        s_Data.CameraBuffer.ViewProjectionMatrix = camera.GetProjectionMatrix() * glm::inverse(transform);
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer3DData::CameraData));

        
        glm::vec3 camPos = glm::vec3(transform[3]);
        SetViewPosition(camPos);
        
        StartBatch();
    }

    void Renderer3D::BeginScene(EditorCamera& camera)
    {
        s_Data.CameraBuffer.ViewProjectionMatrix = camera.GetViewProjection();
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer3DData::CameraData));

        SetViewPosition(camera.GetPosition());
        
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

        const uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CubeVertexBufferPtr - (uint8_t*)s_Data.CubeVertexBufferBase);
        s_Data.CubeVertexBuffer->SetData(s_Data.CubeVertexBufferBase, dataSize);

        GLint currentFBOi = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &currentFBOi);
        const uint32_t currentFBO = (uint32_t)currentFBOi;

        const bool dirShadowPass = s_Data.ShadowValid && currentFBO == s_Data.ShadowFBO;
        const bool pointShadowPass = s_Data.PointShadowValid && currentFBO == s_Data.PointShadowFBO;

        if (dirShadowPass)
        {
            s_Data.ShadowDepthShader->Bind();
            RenderCommand::DrawIndexed(s_Data.CubeVertexArray, s_Data.CubeIndexCount);
            return;
        }
        if (pointShadowPass)
        {
            s_Data.PointShadowDepthShader->Bind();
            RenderCommand::DrawIndexed(s_Data.CubeVertexArray, s_Data.CubeIndexCount);
            return;
        }
        
        for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
            s_Data.TextureSlots[i]->Bind(i);

        s_Data.CubeShader->Bind();

        int32_t samplers[30];
        for (int i = 0; i < 30; i++)
            samplers[i] = i;
        s_Data.CubeShader->SetIntArray("u_textureSamplers", samplers, 30);

        s_Data.CubeShader->SetInt("u_DebugView", Renderer::GetDebugView());

        UploadLightsToShader(s_Data.CubeShader);
        
        if (s_Data.ShadowValid && s_Data.ShadowDepthTexture != 0)
        {
            s_Data.CubeShader->SetInt("u_HasShadowMap", 1);
            s_Data.CubeShader->SetMat4("u_LightSpaceMatrix", s_Data.LightSpaceMatrix);

            const int shadowMapSlot = Renderer3DData::ReservedDirShadowSlot;
            s_Data.CubeShader->SetInt("u_ShadowMap", shadowMapSlot);

            glActiveTexture(GL_TEXTURE0 + shadowMapSlot);
            glBindTexture(GL_TEXTURE_2D, s_Data.ShadowDepthTexture);
        }
        else
            s_Data.CubeShader->SetInt("u_HasShadowMap", 0);

        UploadPointShadowArrayToShader(s_Data.CubeShader);

        RenderCommand::DrawIndexed(s_Data.CubeVertexArray, s_Data.CubeIndexCount);
        // s_Data.Stats.DrawCalls++;
    }

    void Renderer3D::SetViewPosition(const glm::vec3& pos)
    {
        s_Data.ViewPos = pos;
        s_Data.LightsDirty = true;
    }

    void Renderer3D::SetLights(const std::vector<LightGPU>& lights)
    {
        s_Data.LightCount = (int)std::min<size_t>(lights.size(), 16);
        for (int i = 0; i < s_Data.LightCount; i++)
            s_Data.Lights[i] = lights[i];
        s_Data.LightsDirty = true;
    }


    Ref<MeshGPU> Renderer3D::BuildStaticMeshGPU(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices, const Ref<Shader>& shader)
    {
        Ref<MeshGPU> mesh = CreateRef<MeshGPU>();
        mesh->VAO = VertexArray::Create();
        mesh->Shader = shader;

        constexpr uint32_t kFloatsPerVertex = 14;

        std::vector<float> packed;
        packed.reserve(vertices.size() * kFloatsPerVertex);

        for (const MeshVertex& v : vertices)
        {
            packed.push_back(v.Position.x); packed.push_back(v.Position.y); packed.push_back(v.Position.z);
            packed.push_back(v.Normal.x);   packed.push_back(v.Normal.y);   packed.push_back(v.Normal.z);
            packed.push_back(v.UV.x);       packed.push_back(v.UV.y);
        }

        const uint32_t vbSizeBytes = (uint32_t)(packed.size() * sizeof(float));
        Ref<VertexBuffer> vbo = VertexBuffer::Create(packed.data(), vbSizeBytes);

        BufferLayout layout = {
            { "a_Position", ShaderDataType::Float3, false },
            { "a_Normal",   ShaderDataType::Float3, false },
            { "a_TexCoord", ShaderDataType::Float2, false }
        };
        vbo->SetLayout(layout);

        mesh->VAO->AddVertexBuffer(vbo);

        Ref<IndexBuffer> ibo = IndexBuffer::Create((uint32_t*)indices.data(), (uint32_t)indices.size());
        mesh->VAO->SetIndexBuffer(ibo);

        mesh->IndexCount = (uint32_t)indices.size();
        return mesh;
    }
    
    void Renderer3D::DrawMesh(const glm::mat4& transform, MeshRendererComponent& meshRenderer, int entityID)
    {
        // Draw with MeshGPU if available
        if (meshRenderer.Mesh /*&& meshRenderer.Mesh->VAO && meshRenderer.Mesh->Shader && meshRenderer.Mesh->IndexCount > 0*/)
        {
            auto meshGPU = AssetManager::GetAsset<MeshGPU>(meshRenderer.Mesh);
            if (!meshGPU)
                return;
            meshGPU->Shader->Bind();
            meshGPU->Shader->SetInt("u_EntityID", entityID);
            //meshRenderer.Mesh->Shader->SetFloat4("u_Color", meshRenderer.Color);
            meshGPU->Shader->SetMat4("u_ViewProjection", s_Data.CameraBuffer.ViewProjectionMatrix);
            meshGPU->Shader->SetMat4("u_Transform", transform);
            meshGPU->Shader->SetInt("u_DebugView", Renderer::GetDebugView());
            
            if (s_Data.LightsDirty)
            {
                UploadLightsToShader(meshGPU->Shader);
                s_Data.LightsDirty = false;
            }
            
            if (s_Data.ShadowValid && s_Data.ShadowDepthTexture != 0)
            {
                meshGPU->Shader->SetInt("u_HasShadowMap", 1);
                meshGPU->Shader->SetMat4("u_LightSpaceMatrix", s_Data.LightSpaceMatrix);
                meshGPU->Shader->SetFloat("u_ShadowBias", s_Data.ShadowBias);
                
                meshGPU->Shader->SetInt("u_ShadowMap", 31);
                glActiveTexture(GL_TEXTURE0 + 31);
                glBindTexture(GL_TEXTURE_2D, s_Data.ShadowDepthTexture);
            }
            else
                meshGPU->Shader->SetInt("u_HasShadowMap", 0);
            /*if (HasPointShadowMap())
            {
                meshGPU->Shader->SetInt("u_HasPointShadowMap", 1);
                meshGPU->Shader->SetFloat3("u_PointShadowLightPos", GetPointShadowLightPos());
                meshGPU->Shader->SetFloat("u_PointShadowFarPlane", GetPointShadowFarPlane());

                const int pointShadowSlot = 30; // pick a free slot (you used 15 for 2D shadow map)
                meshGPU->Shader->SetInt("u_PointShadowMap", pointShadowSlot);

                glActiveTexture(GL_TEXTURE0 + pointShadowSlot);
                glBindTexture(GL_TEXTURE_CUBE_MAP, GetPointShadowMapRendererID());
            }
            else
            {
                meshGPU->Shader->SetInt("u_HasPointShadowMap", 0);
            }*/
            UploadPointShadowArrayToShader(meshGPU->Shader);

            
            if (!meshGPU->Submeshes.empty())
            {
                for (const auto& sm : meshGPU->Submeshes)
                {
                    if (sm.IndexCount == 0)
                        continue;
            
                    const uint32_t slot = sm.MaterialIndex;
                    
                    AssetHandle matHandle = 0;
                    if (slot < meshRenderer.MaterialHandleOverrides.size())
                        matHandle = meshRenderer.MaterialHandleOverrides[slot];
                    if (matHandle != 0)
                    {
                        Ref<HMaterial> mat = AssetManager::GetAsset<HMaterial>(matHandle);
                        if (mat)
                            mat->Apply(meshGPU->Shader);
                        else
                        {
                            meshGPU->Shader->SetInt("u_HasAlbedo", 0);
                            meshGPU->Shader->SetFloat4("u_Color", meshRenderer.Color);
                        }
                    }
                    else
                    {
                        meshGPU->Shader->SetInt("u_HasAlbedo", 0);
                        meshGPU->Shader->SetFloat4("u_Color", meshRenderer.Color);
                    }

                    RenderCommand::DrawIndexed(meshGPU->VAO, sm.IndexCount, sm.IndexOffset);
                }
            }
            else
            {
                meshGPU->Shader->SetInt("u_HasAlbedo", 0);
                meshGPU->Shader->SetFloat4("u_Color", meshRenderer.Color);
                RenderCommand::DrawIndexed(meshGPU->VAO, meshGPU->IndexCount);
            }
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
            Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(meshRenderer.Texture);
            for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
            {
                if (*s_Data.TextureSlots[i] == *texture)
                {
                    textureIndex = (float)i;
                    break;
                }
            }
            if (textureIndex == 0.0f)
            {
                if (s_Data.TextureSlotIndex >= s_Data.MaxUserTextureSlots/*MaxTextureSlots*/)
                {
                    Flush();
                    StartBatch();
                }
                textureIndex = (float)s_Data.TextureSlotIndex;
                s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
                s_Data.TextureSlotIndex++;
            }
        }
        
        const glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(transform)));
        for (size_t i = 0; i < 24; i++)
        {
            const glm::vec4 worldPos4 = transform * s_Data.VertexPos[i];

            s_Data.CubeVertexBufferPtr->Position = glm::vec3(worldPos4);
            s_Data.CubeVertexBufferPtr->Normal = glm::normalize(normalMat * s_Data.VertexNormal[i]);
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

    void Renderer3D::BeginShadowPass(const glm::vec3& lightDirection, const glm::vec3& focusPosition)
    {
        CreateShadowResources();
            
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &s_Data.OldFBO);
        glGetIntegerv(GL_VIEWPORT, s_Data.OldViewport);
            
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
            
        const glm::vec3 dir = glm::normalize(lightDirection);
        s_Data.ShadowLightDir = dir;
            
        const float orthoRange = 120.0f;
        const float nearPlane = -200.0f;
        const float farPlane = 200.0f;
        
        const glm::vec3 up = (glm::abs(dir.y) > 0.99f) ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
            
        const glm::vec3 lightPos = focusPosition - dir * 30.0f;

        const glm::mat4 lightView = glm::lookAt(lightPos, focusPosition, up);
        const glm::mat4 lightProj = glm::ortho(-orthoRange, orthoRange, -orthoRange, orthoRange, nearPlane, farPlane);

        s_Data.LightSpaceMatrix = lightProj * lightView;
            
        glViewport(0, 0, (GLsizei)s_Data.ShadowMapSize, (GLsizei)s_Data.ShadowMapSize);
        glBindFramebuffer(GL_FRAMEBUFFER, s_Data.ShadowFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
            
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
            
        s_Data.ShadowDepthShader->Bind();
        s_Data.ShadowDepthShader->SetMat4("u_LightSpaceMatrix", s_Data.LightSpaceMatrix);

        s_Data.ShadowValid = true;
            
        s_Data.CameraBuffer.ViewProjectionMatrix = s_Data.LightSpaceMatrix;
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer3DData::CameraData));

        StartBatch();
    }


    void Renderer3D::EndShadowPass()
    {
        Flush(); 

        glBindFramebuffer(GL_FRAMEBUFFER, s_Data.OldFBO);
        glViewport(s_Data.OldViewport[0], s_Data.OldViewport[1], s_Data.OldViewport[2], s_Data.OldViewport[3]);

        glCullFace(GL_BACK);
    }

    void Renderer3D::DrawMeshShadow(const glm::mat4& transform, MeshRendererComponent& meshRenderer)
    {
        if (!s_Data.ShadowValid)
            return;
        
        if (meshRenderer.Mesh)
        {
            auto meshGPU = AssetManager::GetAsset<MeshGPU>(meshRenderer.Mesh);
            if (!meshGPU || !meshGPU->VAO)
                return;

            s_Data.ShadowDepthShader->Bind();
            s_Data.ShadowDepthShader->SetMat4("u_Transform", transform);

            if (!meshGPU->Submeshes.empty())
                for (const auto& sm : meshGPU->Submeshes)
                {
                    if (sm.IndexCount == 0)
                        continue;
                    RenderCommand::DrawIndexed(meshGPU->VAO, sm.IndexCount, sm.IndexOffset);
                }
            else
                RenderCommand::DrawIndexed(meshGPU->VAO, meshGPU->IndexCount);
            return;
        }
        
        /*if (s_Data.CubeIndexCount >= s_Data.MaxIndices)*/if (s_Data.TextureSlotIndex >= s_Data.MaxTextureSlots)
        {
            Flush();
            StartBatch();
        }
        
        float textureIndex = 0.0f;
        
        for (size_t i = 0; i < 24; i++)
        {
            glm::vec4 worldPos4 = transform * s_Data.VertexPos[i];
            
            s_Data.CubeVertexBufferPtr->Position = glm::vec3(worldPos4);
            
            glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(transform)));
            s_Data.CubeVertexBufferPtr->Normal = glm::normalize(normalMat * s_Data.VertexNormal[i]);

            s_Data.CubeVertexBufferPtr->Color = meshRenderer.Color;
            s_Data.CubeVertexBufferPtr->TexCoord = s_Data.VertexUV[i];
            s_Data.CubeVertexBufferPtr->TexIndex = textureIndex;
            s_Data.CubeVertexBufferPtr->TilingFactor = meshRenderer.TilingFactor;
            s_Data.CubeVertexBufferPtr->EntityID = -1;

            s_Data.CubeVertexBufferPtr++;
        }

        s_Data.CubeIndexCount += 36;
    }
    
    void Renderer3D::DrawMeshPointShadow(const glm::mat4& transform, MeshRendererComponent& meshRenderer)
    {
        if (!s_Data.PointShadowValid)
            return;

        s_Data.PointShadowDepthShader->Bind();
        s_Data.PointShadowDepthShader->SetMat4("u_Model", transform); 
        
        if (meshRenderer.Mesh)
        {
            auto meshGPU = AssetManager::GetAsset<MeshGPU>(meshRenderer.Mesh);
            if (!meshGPU || !meshGPU->VAO)
                return;

            if (!meshGPU->Submeshes.empty())
            {
                for (const auto& sm : meshGPU->Submeshes)
                {
                    if (sm.IndexCount == 0) continue;
                    RenderCommand::DrawIndexed(meshGPU->VAO, sm.IndexCount, sm.IndexOffset);
                }
            }
            else
            {
                RenderCommand::DrawIndexed(meshGPU->VAO, meshGPU->IndexCount);
            }
            return; 
        }
        
        CubeVertex* ptr = s_Data.CubeVertexBufferBase;
        for (size_t i = 0; i < 24; i++)
        {
            glm::vec4 wp = transform * s_Data.VertexPos[i];
            ptr->Position = glm::vec3(wp);

            ptr->Normal = glm::vec3(0.0f); 
            ptr->Color = glm::vec4(0.0f);
            ptr->TexCoord = glm::vec2(0.0f);
            ptr->TexIndex = 0.0f;
            ptr->TilingFactor = 1.0f;
            ptr->EntityID = -1;

            ptr++;
        }

        s_Data.CubeVertexBuffer->SetData(s_Data.CubeVertexBufferBase, 24 * sizeof(CubeVertex));

        RenderCommand::DrawIndexed(s_Data.CubeVertexArray, 36);
    }
    
    void Renderer3D::BeginPointShadowAtlas()
    {
        CreatePointShadowResources();

        // reset
        for (int i = 0; i < 16; i++)
        {
            s_Data.PointShadowIndex[i] = -1;
            s_Data.PointShadowLightPos[i] = glm::vec3(0.0f);
            s_Data.PointShadowFarPlane[i] = 1.0f;
        }

        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &s_Data.OldFBO);
        glGetIntegerv(GL_VIEWPORT, s_Data.OldViewport);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glViewport(0, 0, (GLsizei)s_Data.PointShadowMapSize, (GLsizei)s_Data.PointShadowMapSize);
        glBindFramebuffer(GL_FRAMEBUFFER, s_Data.PointShadowFBO);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        s_Data.PointShadowValid = true;
    }


    void Renderer3D::BeginPointShadowCaster(uint32_t casterIndex, int lightIndex,
        const glm::vec3& lightPosition, float farPlane)
    {
        if (casterIndex >= s_Data.MaxPointShadowCasters)
            return;
        if (lightIndex < 0 || lightIndex >= 16)
            return;

        const int layerOffset = (int)casterIndex * 6;

        // Clear only this caster’s 6 layers
        const float clearDepth = 1.0f;
        glClearTexSubImage(s_Data.PointShadowDepthCubemapArray, 0,
            0, 0, layerOffset,
            (GLsizei)s_Data.PointShadowMapSize, (GLsizei)s_Data.PointShadowMapSize, 6,
            GL_DEPTH_COMPONENT, GL_FLOAT, &clearDepth);

        // Build shadow matrices
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, farPlane);
        glm::mat4 shadowTransforms[6] = {
            shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3( 1, 0, 0), glm::vec3(0,-1, 0)),
            shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3(-1, 0, 0), glm::vec3(0,-1, 0)),
            shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3( 0, 1, 0), glm::vec3(0, 0, 1)),
            shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3( 0,-1, 0), glm::vec3(0, 0,-1)),
            shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3( 0, 0, 1), glm::vec3(0,-1, 0)),
            shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3( 0, 0,-1), glm::vec3(0,-1, 0))
        };

        s_Data.PointShadowDepthShader->Bind();
        for (int i = 0; i < 6; i++)
            s_Data.PointShadowDepthShader->SetMat4("u_ShadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);

        s_Data.PointShadowDepthShader->SetFloat3("u_LightPos", lightPosition);
        s_Data.PointShadowDepthShader->SetFloat("u_FarPlane", farPlane);
        s_Data.PointShadowDepthShader->SetInt("u_LayerOffset", layerOffset);

        // Store per-light lookup for shading
        s_Data.PointShadowIndex[lightIndex] = (int)casterIndex;
        s_Data.PointShadowLightPos[lightIndex] = lightPosition;
        s_Data.PointShadowFarPlane[lightIndex] = farPlane;

        StartBatch();
    }
}
