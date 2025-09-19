

#include "UUID.h"
#include <random>


namespace HRealEngine
{
    static std::random_device s_RandomDevice;
    static std::mt19937_64 s_RandomGenerator(s_RandomDevice());
    static std::uniform_int_distribution<uint64_t> s_UniformDistribution;


    UUID::UUID() : m_UUID(s_UniformDistribution(s_RandomGenerator))
    {
        
    }

    UUID::UUID(uint64_t uuid) : m_UUID(uuid)
    {
        
    }
}
