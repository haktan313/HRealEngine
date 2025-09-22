
#pragma once

#ifdef HREALENGINE_PLATFORM_WINDOWS

extern HRealEngine::Application* HRealEngine::CreateApplication(AppCommandLineArgs args);/*This function is not defined here, but trust me, it will be defined in another compilation unit.
when you build the whole solution (engine + Sandbox), the linker finds Sandbox’s definition and connects it.*/

int main(int argc, char** argv)
{
	HRealEngine::Logger::Init();
	
	auto app = HRealEngine::CreateApplication({argc, argv});
	app->Run();
	delete app;
	return 0;
}

#endif