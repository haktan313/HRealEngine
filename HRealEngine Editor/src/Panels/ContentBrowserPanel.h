#pragma once

#include <filesystem>
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
    private:
        void ImportOBJ();
        std::filesystem::path MakeUniquePath(const std::filesystem::path& p) const;
        
        std::filesystem::path m_CurrentDirectory;

        Ref<Texture2D> m_FileIcon, m_FolderIcon;

        std::string m_LastError;
        bool m_OpenErrorPopup = false;

    };
}
