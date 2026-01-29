#pragma once
#include <filesystem>

#include "../../../../HRealEngine Editor/src/Panels/ContentBrowserPanel.h"
#include "HRealEngine/Asset/AssetManagerBase.h"
#include "HRealEngine/Asset/EditorAssetManager.h"


namespace HRealEngine
{
    struct ProjectConfig
    {
        std::string Name = "Untitled";

        AssetHandle StartScene;

        std::filesystem::path AssetDirectory;
        std::filesystem::path AssetRegistryPath;
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
        static std::filesystem::path GetAssetRegistryPath()
        {
            HREALENGINE_CORE_DEBUGBREAK(s_ActiveProject, "No active project!");
            return GetAssetDirectory() / s_ActiveProject->m_Config.AssetRegistryPath;
        }
        static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
        {
            HREALENGINE_CORE_DEBUGBREAK(s_ActiveProject, "No active project!");
            return GetAssetDirectory() / path;
        }
        
        ProjectConfig& GetConfig() { return m_Config; }
        Ref<AssetManagerBase> GetAssetManager() { return m_AssetManager; }
        Ref<EditorAssetManager> GetEditorAssetManager() { return std::static_pointer_cast<EditorAssetManager>(m_AssetManager); }
        
        static Ref<Project> GetActive() { return s_ActiveProject; }
        static ContentBrowserPanel* GetContentBrowserPanel() { return m_ContentBrowserPanel; }

        static Ref<Project> New(const std::filesystem::path& projectFilePath);
        static Ref<Project> New();
        static Ref<Project> Load(const std::filesystem::path& path);
        static bool SaveActive(const std::filesystem::path& path);
        static void SetContentBrowserPanel(ContentBrowserPanel* panel) { m_ContentBrowserPanel = panel; }

        static bool CreateFromTemplate(const std::filesystem::path& templateDir, const std::filesystem::path& destinationDir,
            const std::string& projectName, std::string* outError = nullptr);
    private:
        static bool CopyDirectoryRecursive(const std::filesystem::path& src, const std::filesystem::path& dst, std::string* outError);
        static bool ShouldSkipPath(const std::filesystem::path& p);
        ProjectConfig m_Config;
        std::filesystem::path m_ProjectDirectory;
        Ref<AssetManagerBase> m_AssetManager;

        inline static ContentBrowserPanel* m_ContentBrowserPanel;
        inline static Ref<Project> s_ActiveProject;
    };
}
