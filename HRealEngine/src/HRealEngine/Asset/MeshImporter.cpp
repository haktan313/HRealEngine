#include "HRpch.h"
#include "MeshImporter.h"

#include "AssetManager.h"
#include "HRealEngine/Core/MeshLoader.h"

namespace HRealEngine
{
    Ref<Asset> MeshImporter::ImportMesh(AssetHandle assetHandle, const AssetMetadata& metaData)
    {
        static Ref<Shader> s_DefaultMeshShader = Shader::Create("assets/shaders/StaticMesh.glsl");
        
        std::string ext = metaData.FilePath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        LOG_CORE_INFO("Importing mesh asset from path: {} (ext: {})", metaData.FilePath.string(), ext);

        Ref<Asset> asset = nullptr;

        if (ext == ".hmesh")
        {
            asset = MeshLoader::LoadHMeshAsset(metaData.FilePath, Project::GetActive()->GetAssetDirectory(), s_DefaultMeshShader);
        }
        else if (ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb")
        {
            //Project::GetContentBrowserPanel()->ImportOBJ(Project::GetAssetFileSystemPath(metaData.FilePath));
            asset = MeshLoader::LoadHMeshAsset(metaData.FilePath, Project::GetActive()->GetAssetDirectory(), s_DefaultMeshShader);
        }
        else
        {
            LOG_CORE_WARN("Unsupported mesh extension: {}", ext);
        }
        return asset;
    }
}
