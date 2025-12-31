#pragma once

#include <filesystem>
#include <map>

#include "HRealEngine/Core/Core.h"
#include "HRealEngine/Renderer/Texture.h"

namespace HRealEngine
{
    class ContentBrowserPanel
    {
    public:
        ContentBrowserPanel();
        ~ContentBrowserPanel() = default;

        void OnImGuiRender();
        void RefreshAssetTree();
        void ImportOBJ(const std::filesystem::path& srcObj);
    private:
        void ImportOBJ();
        void CreateDirectoriesIfNotExists(const std::filesystem::path& srcObj, std::filesystem::path& dstObj, std::filesystem::path& lastCopiedTexAbs,
            std::vector<std::filesystem::path>& texturePaths);
        void ImportDataFromOBJ(const std::filesystem::path& dstObj, const std::filesystem::path& lastCopiedTexAbs,
            const std::vector<std::filesystem::path>& texturePaths);
        std::filesystem::path MakeUniquePath(const std::filesystem::path& p) const;
        
        std::filesystem::path m_CurrentDirectory;
        std::filesystem::path m_BaseDirectory;

        Ref<Texture2D> m_FileIcon, m_FolderIcon;

        std::string m_LastError;
        bool m_OpenErrorPopup = false;

        struct TreeNode
        {
            std::filesystem::path Path;
            AssetHandle Handle = 0;
			
            uint32_t Parent = (uint32_t)-1;
            std::map<std::filesystem::path, uint32_t> Children;

            TreeNode(const std::filesystem::path& path, AssetHandle handle) : Path(path), Handle(handle) {}
        };
        std::vector<TreeNode> m_TreeNodes;
        std::map<std::filesystem::path, std::vector<std::filesystem::path>> m_AssetTree;
        enum class Mode
        {
            Asset = 0, FileSystem = 1
        };
        Mode m_Mode = Mode::Asset;
    };
}
