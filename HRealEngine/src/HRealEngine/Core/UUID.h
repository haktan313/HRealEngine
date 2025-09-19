#pragma once

#include <cstdint>
#include <xhash>

namespace HRealEngine
{
    class UUID
    {
    public:
        UUID();
        UUID(uint64_t uuid);
        UUID(const UUID&) = default;
        
        operator uint64_t() const { return m_UUID; }
    private:
        uint64_t m_UUID;

    };
}

namespace std
{
    template<>
    struct hash<HRealEngine::UUID>
    {
        std::size_t operator()(const HRealEngine::UUID& uuid) const
        {
            std::hash<uint64_t> hasherDummy;
            uint64_t id = (uint64_t)uuid;
            return hasherDummy(id);
        }
    };
}



