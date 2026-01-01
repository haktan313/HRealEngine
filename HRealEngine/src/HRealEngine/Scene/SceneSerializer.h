#pragma once
#include "Scene.h"

namespace HRealEngine
{
    class SceneSerializer
    {
    public:
        SceneSerializer(const Ref<Scene>& scene);
        ~SceneSerializer() = default;

        void Serialize(const std::filesystem::path& filepath);
        void SerializeRuntime(const std::filesystem::path& filepath);
        bool Deserialize(const std::filesystem::path& filepath);
        bool DeserializeRuntime(const std::filesystem::path& filepath);
    private:
        Ref<Scene> sceneRef;
    };
}
