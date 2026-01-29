#include "HRpch.h"
#include "Project.h"

#include "ProjectSerializer.h"

namespace HRealEngine
{
    Ref<Project> Project::New(const std::filesystem::path& projectFilePath)
    {
        s_ActiveProject = CreateRef<Project>();
        s_ActiveProject->m_ProjectDirectory = projectFilePath.parent_path();

        auto& config = s_ActiveProject->GetConfig();
        config.Name = projectFilePath.stem().string();

        config.AssetDirectory = "assets";
        config.AssetRegistryPath = "AssetRegistry.yaml";
        config.ScriptModulePath = "Scripts/Binaries/Sandbox.dll";

        auto editorAssetManager = CreateRef<EditorAssetManager>();
        s_ActiveProject->m_AssetManager = editorAssetManager;

        std::filesystem::create_directories(GetAssetDirectory());

        SaveActive(projectFilePath);

        return s_ActiveProject;
    }

    Ref<Project> Project::New()
    {
        s_ActiveProject = CreateRef<Project>();
        return s_ActiveProject;
    }

    Ref<Project> Project::Load(const std::filesystem::path& path)
    {
        Ref<Project> project = CreateRef<Project>();

        ProjectSerializer serializer(project);
        if (serializer.Deserialize(path))
        {
            project->m_ProjectDirectory = path.parent_path();
            s_ActiveProject = project;
            auto editorAssetManager = CreateRef<EditorAssetManager>();
            s_ActiveProject->m_AssetManager = editorAssetManager;
            editorAssetManager->DeserializeAssetRegistry();
            return s_ActiveProject;
        }

        return nullptr;
    }

    bool Project::SaveActive(const std::filesystem::path& path)
    {
        ProjectSerializer serializer(s_ActiveProject);
        if (serializer.Serialize(path))
        {
            s_ActiveProject->m_ProjectDirectory = path.parent_path();
            return true;
        }

        return false;
    }

    bool Project::CreateFromTemplate(const std::filesystem::path& templateDir,
        const std::filesystem::path& destinationDir, const std::string& projectName, std::string* outError)
    {
        (void)projectName;

        std::error_code ec;

        if (!std::filesystem::exists(templateDir, ec))
        {
            if (outError) *outError = "Template directory does not exist: " + templateDir.string();
            return false;
        }
        
        if (std::filesystem::exists(destinationDir, ec))
        {
            if (outError) *outError = "Destination already exists (refusing to overwrite): " + destinationDir.string();
            return false;
        }

        if (!CopyDirectoryRecursive(templateDir, destinationDir, outError))
            return false;

        return true;
    }

    bool Project::CopyDirectoryRecursive(const std::filesystem::path& src, const std::filesystem::path& dst,
        std::string* outError)
    {
        std::error_code ec;

        if (!std::filesystem::exists(src, ec) || !std::filesystem::is_directory(src, ec))
        {
            if (outError) *outError = "Template path is not a directory: " + src.string();
            return false;
        }

        std::filesystem::create_directories(dst, ec);
        if (ec)
        {
            if (outError) *outError = "Failed to create destination directory: " + dst.string() + " (" + ec.message() + ")";
            return false;
        }
        
        for (std::filesystem::recursive_directory_iterator it(src, std::filesystem::directory_options::skip_permission_denied, ec), end; it != end; it.increment(ec))
        {
            if (ec)
            {
                if (outError) *outError = "Directory iteration error: " + ec.message();
                return false;
            }

            const std::filesystem::path current = it->path();
            
            if (ShouldSkipPath(current))
            {
                if (it->is_directory(ec))
                    it.disable_recursion_pending();
                continue;
            }
            
            std::filesystem::path rel = std::filesystem::relative(current, src, ec);
            if (ec)
            {
                if (outError) *outError = "Failed to compute relative path for: " + current.string() + " (" + ec.message() + ")";
                return false;
            }

            std::filesystem::path target = dst / rel;
            
            if (std::filesystem::is_symlink(current, ec))
                continue;

            if (it->is_directory(ec))
            {
                std::filesystem::create_directories(target, ec);
                if (ec)
                {
                    if (outError)
                        *outError = "Failed to create directory: " + target.string() + " (" + ec.message() + ")";
                    return false;
                }
            }
            else if (it->is_regular_file(ec))
            {
                std::filesystem::create_directories(target.parent_path(), ec);
                
                std::filesystem::copy_file(current, target, std::filesystem::copy_options::overwrite_existing, ec);
                if (ec)
                {
                    if (outError) *outError = "Failed to copy file: " + current.string() + " -> " + target.string() + " (" + ec.message() + ")";
                    return false;
                }
            }
        }
        return true;
    }

    bool Project::ShouldSkipPath(const std::filesystem::path& p)
    {
        const std::string name = p.filename().string();

        if (name == ".vs")
            return true;
        if (name == ".idea")
            return true;
        if (name == "bin")
            return true;
        if (name == "bin-int")
            return true;
        if (name == "build")
            return true;
        if (name == ".git")
            return true;
        if (name == ".gitmodules")
            return true;
        if (name == "imgui.ini")
            return true;
        
        return false;
    }
}
