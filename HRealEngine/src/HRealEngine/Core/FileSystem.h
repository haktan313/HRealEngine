#pragma once
#include <filesystem>

#include "Buffer.h"

namespace HRealEngine
{
    class FileSystem
    {
    public:
        static Buffer ReadFileBinary(const std::filesystem::path& filepath);
    };
}
