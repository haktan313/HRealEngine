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
    };
}
