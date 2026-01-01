#include "HRpch.h"
#include "SceneImporter.h"

#include "HRealEngine/Project/Project.h"
#include "HRealEngine/Scene/SceneSerializer.h"

namespace HRealEngine
{
    Ref<Scene> SceneImporter::ImportScene(AssetHandle handle, const AssetMetadata& metadata)
    {
        Ref<Scene> scene = LoadScene(Project::GetAssetDirectory() / metadata.FilePath);
        scene->Handle = handle;
        return scene;
    }

    Ref<Scene> SceneImporter::LoadScene(const std::filesystem::path& path)
    {
        Ref<Scene> scene = CreateRef<Scene>();
        SceneSerializer serializer(scene);
        serializer.Deserialize(path);
        return scene;
    }

    void SceneImporter::SaveScene(Ref<Scene> scene, const std::filesystem::path& path)
    {
        SceneSerializer serializer(scene);
        serializer.Serialize(Project::GetAssetDirectory() / path);
    }
}
