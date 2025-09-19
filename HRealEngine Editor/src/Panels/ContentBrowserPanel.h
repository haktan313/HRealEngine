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
        std::filesystem::path m_CurrentDirectory;

        Ref<Texture2D> m_FileIcon, m_FolderIcon;
    };
}
