#include "HRpch.h"
#include "JoltWorldHelper.h"

#include "HRealEngine/Core/Timestep.h"

#include "Jolt/Jolt.h"
#include "Jolt/Core/Factory.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/RegisterTypes.h"

namespace HRealEngine
{
    void JoltWorldHelper::Initialize(JPH::PhysicsSystem& physics_system)
    {
        if (m_Initialized)
            return;

        // Register allocation hook. In this example we'll just let Jolt use malloc / free but you can override these if you want (see Memory.h).
        // This needs to be done before any other Jolt function is called.
        JPH::RegisterDefaultAllocator();

        // Create a factory, this class is responsible for creating instances of classes based on their name or hash and is mainly used for deserialization of saved data.
        // It is not directly used in this example but still required.
        JPH::Factory::sInstance = new JPH::Factory();
        
        // Register all physics types with the factory and install their collision handlers with the CollisionDispatch class.
        // If you have your own custom shape types you probably need to register their handlers with the CollisionDispatch before calling this function.
        // If you implement your own default material (PhysicsMaterial::sDefault) make sure to initialize it before this function or else this function will create one for you.
        JPH::RegisterTypes();

        // We need a temp allocator for temporary allocations during the physics update. We're
        // pre-allocating 10 MB to avoid having to do allocations during the physics update.
        // B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
        // If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
        // malloc / free.
        m_TempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);

        // Job system: thread pool
        const uint32_t max_jobs = 1024;
        const uint32_t max_barriers = 1024;

        const int thread_count = std::max(1u, std::thread::hardware_concurrency() - 1);
        // We need a job system that will execute physics jobs on multiple threads. Typically
        // you would implement the JobSystem interface yourself and let Jolt Physics run on top
        // of your own job scheduler. JobSystemThreadPool is an example implementation.
        m_JobSystem = std::make_unique<JPH::JobSystemThreadPool>(max_jobs, max_barriers, thread_count);
        
        const uint32_t cMaxBodies = 65536;
        const uint32_t cNumBodyMutexes = 0;
        const uint32_t cMaxBodyPairs = 65536;
        const uint32_t cMaxContactConstraints = 10240;

        physics_system.Init( cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,
            m_BPLayerInterface, m_ObjectVsBroadPhaseLayerFilter, m_ObjectLayerPairFilter);
        physics_system.SetBodyActivationListener(&m_BodyActivationListener);

        m_Initialized = true;
    }

    void JoltWorldHelper::StepWorld(Timestep deltaTime, JPH::PhysicsSystem& physics_system)
    {
        const int collisionSteps = 1;
        physics_system.Update(deltaTime, collisionSteps, m_TempAllocator.get(), m_JobSystem.get());
    }
}
