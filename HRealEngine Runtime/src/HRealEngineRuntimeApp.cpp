
#include "HRealEngine.h"
#include "RuntimeLayer.h"
#include "HRealEngine/Core/EntryPoint.h"

namespace HRealEngine
{
    class HRealEngineRuntimeApp : public Application
    {
    public:
        HRealEngineRuntimeApp(const ApplicationSpecification& spec) : Application(spec)
        {
            PushLayer(new RuntimeLayer());
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
