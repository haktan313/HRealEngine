
#include "GameLayer.h"
#include "HRealEngine.h"
#include "HRealEngine/Core/EntryPoint.h"

namespace HRealEngine
{
    class FlappyMourinho : public Application
    {
    public:
        FlappyMourinho(const ApplicationSpecification& spec) : Application(spec)
        {
            PushLayer(new GameLayer());
        }
    };

    Application* CreateApplication(AppCommandLineArgs args)
    {
        ApplicationSpecification spec;
        spec.Name = "FlappyMourinho";
        spec.CommandLineArgs = args;
        
        return new FlappyMourinho(spec); 
    }
}