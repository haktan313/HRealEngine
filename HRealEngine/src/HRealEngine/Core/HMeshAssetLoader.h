#pragma once
#include <filesystem>
#include "HRealEngine/Core/Core.h"
#include "HRealEngine/Renderer/Mesh.h"
#include "HRealEngine/Renderer/Shader.h"

namespace HRealEngine
{
    Ref<MeshGPU> LoadHMeshAsset(const std::filesystem::path& hmeshPath, const std::filesystem::path& assetsRoot, const Ref<Shader>& shader);
}
