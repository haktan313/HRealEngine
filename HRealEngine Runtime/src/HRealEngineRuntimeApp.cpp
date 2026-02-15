
#include "HRealEngine.h"
#include "RuntimeLayer.h"
#include "HRealEngine/Core/EntryPoint.h"
#include "HRealEngine/Scripting/ScriptEngine.h"

namespace HRealEngine
{
    class HRealEngineRuntimeApp : public Application
    {
        public:
        HRealEngineRuntimeApp(const ApplicationSpecification& spec) : Application(spec)
        {
            if (!LoadProjectFromCommandLine())
            {
                LOG_CORE_ERROR("[Runtime] Failed to load project!");
                return;
            }
            ScriptEngine::Init();
            PushLayer(new RuntimeLayer());
        }
        ~HRealEngineRuntimeApp()
        {
        }
    private:
        bool LoadProjectFromCommandLine()
        {
            auto args = GetSpecification().CommandLineArgs;
            std::filesystem::path projectPath;
            
            if (args.Count > 1)
            {
                projectPath = args[1];
                LOG_CORE_INFO("[Runtime] Using command line project: {}", projectPath.string());
            }
            else
            {
                LOG_CORE_INFO("[Runtime] No command line argument, searching current directory: {}", std::filesystem::current_path().string());
                try
                {
                    for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path()))
                        if (entry.is_regular_file() && entry.path().extension() == ".hrpj")
                        {
                            projectPath = entry.path();
                            LOG_CORE_INFO("[Runtime] Found project file: {}", projectPath.filename().string());
                            break;
                        }
                }
                catch (const std::exception& e)
                {
                    LOG_CORE_ERROR("[Runtime] Error searching for .hrpj: {}", e.what());
                }
            }

            if (projectPath.empty())
            {
                LOG_CORE_ERROR("[Runtime] No .hrpj file found in directory: {}", std::filesystem::current_path().string());
                return false;
            }

            if (!std::filesystem::exists(projectPath))
            {
                LOG_CORE_ERROR("[Runtime] Project file does not exist: {}", projectPath.string());
                return false;
            }

            LOG_CORE_INFO("[Runtime] Loading project: {}", projectPath.string());
            if (!Project::Load(projectPath))
            {
                LOG_CORE_ERROR("[Runtime] Failed to load project: {}", projectPath.string());
                return false;
            }

            LOG_CORE_INFO("[Runtime] Successfully loaded project: '{}'", Project::GetActive()->GetConfig().Name);

            AssetHandle startScene = Project::GetActive()->GetConfig().StartScene;
            if (!startScene)
                LOG_CORE_WARN("[Runtime] Warning: Project has no StartScene set!");
            else
                LOG_CORE_INFO("[Runtime] StartScene handle: {}", (uint64_t)startScene);

            return true;
        }
    };
    
    Application* CreateApplication(AppCommandLineArgs args)
    {
        ApplicationSpecification spec;
        spec.Name = "HRealEngine Runtime";
        spec.CommandLineArgs = args;
        spec.EditorAssetsPath = "assets";

        return new HRealEngineRuntimeApp(spec);
    }
}
