#pragma once
#include <glm/glm.hpp>
#include "HRealEngine/Core/Core.h"
#include "HRealEngine/Renderer/VertexArray.h"
#include "HRealEngine/Renderer/Shader.h"

namespace HRealEngine
{
    struct MeshGPU
    {
        Ref<VertexArray> VAO;
        Ref<Shader> Shader;
        uint32_t IndexCount = 0;
    };
}
