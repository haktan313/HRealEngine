#pragma once
#include <filesystem>

namespace HRealEngine
{
    class ScriptEngine
    {
    public:
        static void Init();
        static void Shutdown();

        static void LoadAssembly(const std::string& assemblyPath);
    private:
        static void InitMono();
        static void ShutdownMono();
    };
}
