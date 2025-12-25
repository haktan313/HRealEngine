#pragma once
#include <filesystem>

namespace HRealEngine
{
    struct ProjectConfig
    {
        std::string Name = "Untitled";

        std::filesystem::path StartScene;

        std::filesystem::path AssetDirectory;
        std::filesystem::path ScriptModulePath;
    };
    
    class Project
    {
    public:
        static const std::filesystem::path& GetProjectDirectory()
        {
            HREALENGINE_CORE_DEBUGBREAK(s_ActiveProject, "No active project!");
            return s_ActiveProject->m_ProjectDirectory;
        }
        static std::filesystem::path GetAssetDirectory()
        {
            HREALENGINE_CORE_DEBUGBREAK(s_ActiveProject, "No active project!");
            return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
        }
        static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
        {
            HREALENGINE_CORE_DEBUGBREAK(s_ActiveProject, "No active project!");
            return GetAssetDirectory() / path;
        }
        
        ProjectConfig& GetConfig() { return m_Config; }

        static Ref<Project> GetActive() { return s_ActiveProject; }

        static Ref<Project> New();
        static Ref<Project> Load(const std::filesystem::path& path);
        static bool SaveActive(const std::filesystem::path& path);
    private:
        ProjectConfig m_Config;
        std::filesystem::path m_ProjectDirectory;

        inline static Ref<Project> s_ActiveProject;
    };
}
