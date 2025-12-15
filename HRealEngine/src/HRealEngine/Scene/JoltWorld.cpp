#include "HRpch.h"
#include "JoltWorld.h"

#include "RegisterTypes.h"
#include "Core/Factory.h"
#include "Core/TempAllocator.h"

namespace HRealEngine
{
    static void TraceImpl(const char* inFMT, ...)
    {
        char buffer[2048];

        va_list args;
        va_start(args, inFMT);
        vsnprintf(buffer, sizeof(buffer), inFMT, args);
        va_end(args);
        
        LOG_CORE_TRACE("[Jolt] {}", buffer);
    }

    static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, JPH::uint inLine)
    {
        LOG_CORE_ERROR("[Jolt ASSERT] Expr: {} | Msg: {} | File: {}:{}",
            inExpression ? inExpression : "(null)",
            inMessage ? inMessage : "(null)",
            inFile ? inFile : "(null)",
            (uint32_t)inLine);

#if defined(HREALENGINE_DEBUG)
#if defined(_MSC_VER)
        __debugbreak();
#endif
#endif
        return true;
    }

    
    JoltWorld::JoltWorld(Scene* scene) : m_Scene(scene)
    {
    }

    JoltWorld::~JoltWorld()
    {
        
    }

    void JoltWorld::Init()
    {
        JPH::RegisterDefaultAllocator();

        JPH::Trace = TraceImpl;
        JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)
        
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();

        JPH::TempAllocatorImpl temp_allocator(10 * 1024 * 1024);
        LOG_CORE_INFO("JoltWorld initialized (Trace/Assert hooked).");
    }
}
