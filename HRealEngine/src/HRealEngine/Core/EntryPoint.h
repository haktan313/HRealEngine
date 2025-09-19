
//EntryPoint.h
#pragma once

#ifdef HREALENGINE_PLATFORM_WINDOWS

extern HRealEngine::Application* HRealEngine::CreateApplication();/*This function is not defined here, but trust me, it will be defined in another compilation unit.
when you build the whole solution (engine + Sandbox), the linker finds Sandbox’s definition and connects it.*/

int main(int argc, char** argv)
{
	HRealEngine::Logger::Init();
	
	HR_PROFILE_BEGIN_SESSION("Startup", "HRealEngineProfile-Startup.json");
	auto app = HRealEngine::CreateApplication();
	HR_PROFILE_END_SESSION();
	
	HR_PROFILE_BEGIN_SESSION("Runtime", "HRealEngineProfile-Runtime.json");
	app->Run();
	HR_PROFILE_END_SESSION();
	
	HR_PROFILE_BEGIN_SESSION("Shutdown", "HRealEngineProfile-Shutdown.json");
	delete app;
	HR_PROFILE_END_SESSION();
	
	return 0;
}

#endif