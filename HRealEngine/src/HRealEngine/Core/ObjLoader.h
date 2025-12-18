#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "HRealEngine/Core/Core.h"
#include "HRealEngine/Renderer/Mesh.h"
#include "HRealEngine/Renderer/Shader.h"
#include "HRealEngine/Renderer/VertexArray.h"

namespace HRealEngine
{
    struct MeshVertex
    {
        glm::vec3 Position{0.0f};
        glm::vec3 Normal  {0.0f, 1.0f, 0.0f};
        glm::vec2 UV      {0.0f};
        glm::vec3 Tangent {1.0f, 0.0f, 0.0f};
        glm::vec3 Color   {1.0f};
    };
    class ObjLoader
    {
    public:
        static bool LoadMeshFromFile(const std::string& path, std::vector<MeshVertex>& outVertices, std::vector<uint32_t>& outIndices);
        static Ref<MeshGPU> BuildStaticMeshGPU(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices, const Ref<Shader>& shader);
    };
}
