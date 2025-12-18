#include "HRpch.h"
#include "ObjLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "HRealEngine/Renderer/Buffers.h"

namespace HRealEngine
{
    namespace
    {
        glm::vec3 ToVec3(const aiVector3D& v) { return { v.x, v.y, v.z }; }
        glm::vec2 ToVec2(const aiVector3D& v) { return { v.x, v.y }; }
    }

    bool ObjLoader::LoadMeshFromFile(const std::string& path, std::vector<MeshVertex>& outVertices, std::vector<uint32_t>& outIndices)
    {
        Assimp::Importer importer;

        const unsigned flags =
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_GenSmoothNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_ImproveCacheLocality;

        const aiScene* scene = importer.ReadFile(path.c_str(), flags);

        if (!scene || !scene->HasMeshes())
        {
            std::cerr << "Assimp failed: " << importer.GetErrorString() << "\n";
            return false;
        }

        outVertices.clear();
        outIndices.clear();

        for (unsigned m = 0; m < scene->mNumMeshes; ++m)
        {
            aiMesh* mesh = scene->mMeshes[m];
            const uint32_t baseVertex = (uint32_t)outVertices.size();

            const bool hasNormals  = mesh->HasNormals();
            const bool hasUV0      = mesh->HasTextureCoords(0);
            const bool hasTangents = mesh->HasTangentsAndBitangents();
            const bool hasColors0  = mesh->HasVertexColors(0);

            outVertices.reserve(outVertices.size() + mesh->mNumVertices);

            for (unsigned i = 0; i < mesh->mNumVertices; ++i)
            {
                MeshVertex v{};
                v.Position = ToVec3(mesh->mVertices[i]);
                v.Normal = hasNormals ? ToVec3(mesh->mNormals[i]) : glm::vec3(0,1,0);
                v.UV = hasUV0 ? ToVec2(mesh->mTextureCoords[0][i]) : glm::vec2(0);

                if (hasTangents)
                    v.Tangent = ToVec3(mesh->mTangents[i]);

                v.Color = glm::vec3(1.0f);
                if (hasColors0)
                {
                    const aiColor4D c = mesh->mColors[0][i];
                    v.Color = glm::vec3(c.r, c.g, c.b);
                }

                outVertices.push_back(v);
            }

            for (unsigned f = 0; f < mesh->mNumFaces; ++f)
            {
                const aiFace& face = mesh->mFaces[f];
                if (face.mNumIndices != 3) continue;

                outIndices.push_back(baseVertex + (uint32_t)face.mIndices[0]);
                outIndices.push_back(baseVertex + (uint32_t)face.mIndices[1]);
                outIndices.push_back(baseVertex + (uint32_t)face.mIndices[2]);
            }
        }

        std::cout << "Loaded: " << path << " | V: " << outVertices.size() << " | I: " << outIndices.size() << "\n";
        return true;
    }

    Ref<MeshGPU> ObjLoader::BuildStaticMeshGPU(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices, const Ref<Shader>& shader)
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
            packed.push_back(v.Tangent.x);  packed.push_back(v.Tangent.y);  packed.push_back(v.Tangent.z);
            packed.push_back(v.Color.x);    packed.push_back(v.Color.y);    packed.push_back(v.Color.z);
        }

        const uint32_t vbSizeBytes = (uint32_t)(packed.size() * sizeof(float));
        Ref<VertexBuffer> vbo = VertexBuffer::Create(packed.data(), vbSizeBytes);

        BufferLayout layout = {
            { "a_Position", ShaderDataType::Float3, false },
            { "a_Normal",   ShaderDataType::Float3, false },
            { "a_TexCoord", ShaderDataType::Float2, false },
            { "a_Tangent",  ShaderDataType::Float3, false },
            { "a_Color",    ShaderDataType::Float3, false }
        };
        vbo->SetLayout(layout);

        mesh->VAO->AddVertexBuffer(vbo);

        Ref<IndexBuffer> ibo = IndexBuffer::Create((uint32_t*)indices.data(), (uint32_t)indices.size());
        mesh->VAO->SetIndexBuffer(ibo);

        mesh->IndexCount = (uint32_t)indices.size();
        return mesh;
    }

}
