#pragma once
#include <string>

namespace HRealEngine
{
    class FileDialogs
    {
    public:
        static std::string OpenFile(const char* filter);
        static std::string SaveFile(const char* filter);
    };
    class Time
    {
    public:
        static float GetTime();
        static float GetDeltaTime() { return s_DeltaTime; }
        static void SetDeltaTime(float delta) { s_DeltaTime = delta; }
    private:
        static float s_DeltaTime;
    };
}
