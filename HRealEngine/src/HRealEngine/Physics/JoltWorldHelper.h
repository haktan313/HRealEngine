#pragma once
#include <memory>

#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h"
#include "Jolt/Physics/Collision/ObjectLayer.h"
#include "Jolt/Physics/Body/BodyActivationListener.h"
#include "Jolt/Physics/Collision/ContactListener.h"

namespace HRealEngine
{
    class Timestep;
    class JoltWorld;

    using uint = unsigned int;

    // Layer that objects can be in, determines which other objects it can collide with
    // Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
    // layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
    // but only if you do collision testing).
    namespace Layers
    {
        static constexpr JPH::ObjectLayer NON_MOVING = 0;
        static constexpr JPH::ObjectLayer MOVING = 1;
        static constexpr JPH::ObjectLayer PERCEPTION = 2;
        static constexpr JPH::ObjectLayer PERCEIVABLE = 3;
        static constexpr JPH::ObjectLayer NUM_LAYERS = 4;
    };

    // Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
    // a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
    // You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
    // many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
    // your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
    namespace BroadPhaseLayers
    {
        static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        static constexpr JPH::BroadPhaseLayer MOVING(1);
        static constexpr JPH::BroadPhaseLayer PERCEPTION(2);
        static constexpr JPH::BroadPhaseLayer PERCEIVABLE(3);
        static constexpr uint NUM_LAYERS(4);
    };

    
    // BroadPhaseLayerInterface implementation
    // This defines a mapping between object and broadphase layers.
    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
    public:
        BPLayerInterfaceImpl()
        {
            // Create a mapping table from object to broad phase layer
            mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
            mObjectToBroadPhase[Layers::PERCEPTION] = BroadPhaseLayers::PERCEPTION;
            mObjectToBroadPhase[Layers::PERCEIVABLE] = BroadPhaseLayers::PERCEIVABLE;
        }

        virtual uint					GetNumBroadPhaseLayers() const override
        {
            return BroadPhaseLayers::NUM_LAYERS;
        }

        virtual JPH::BroadPhaseLayer			GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
        {
            JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
            return mObjectToBroadPhase[inLayer];
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        virtual const char *			GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
        {
            switch ((BroadPhaseLayer::Type)inLayer)
            {
            case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
            case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
            case (BroadPhaseLayer::Type)BroadPhaseLayers::PERCEPTION:	return "PERCEPTION";
            case (BroadPhaseLayer::Type)BroadPhaseLayers::PERCEIVABLE:	return "PERCEIVABLE";
            default:													JPH_ASSERT(false); return "INVALID";
            }
        }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

    private:
        JPH::BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
    };

    /// Class that determines if an object layer can collide with a broadphase layer
    class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        virtual bool				ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
        {
            switch (inLayer1)
            {
            case Layers::NON_MOVING:
                return inLayer2 == BroadPhaseLayers::MOVING;
            case Layers::MOVING:
                return inLayer2 == BroadPhaseLayers::NON_MOVING 
                    || inLayer2 == BroadPhaseLayers::MOVING;
            case Layers::PERCEPTION:
                return inLayer2 == BroadPhaseLayers::PERCEIVABLE;
            case Layers::PERCEIVABLE:
                return inLayer2 == BroadPhaseLayers::PERCEPTION;
            default:
                JPH_ASSERT(false);
                return false;
            }
        }
    };

    /// Class that determines if two object layers can collide
    class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
    {
    public:
        virtual bool					ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
        {
            switch (inObject1)
            {
            case Layers::NON_MOVING:
                return inObject2 == Layers::MOVING; // Non moving only collides with moving
            case Layers::MOVING:
                return inObject2 == Layers::NON_MOVING 
                    || inObject2 == Layers::MOVING;
            case Layers::PERCEPTION:
                return inObject2 == Layers::PERCEIVABLE;
            case Layers::PERCEIVABLE:
                return inObject2 == Layers::PERCEPTION;
            default:
                JPH_ASSERT(false);
                return false;
            }
        }
    };

    // An example activation listener
    class MyBodyActivationListener : public JPH::BodyActivationListener
    {
    public:
        virtual void		OnBodyActivated(const JPH::BodyID &inBodyID, JPH::uint64 inBodyUserData) override
        {
            std::cout << "A body got activated" << std::endl;
        }

        virtual void		OnBodyDeactivated(const JPH::BodyID &inBodyID, JPH::uint64 inBodyUserData) override
        {
            std::cout << "A body went to sleep" << std::endl;
        }
    };

    class JoltWorldHelper
    {
    public:
        JoltWorldHelper(JoltWorld* joltWorld) : m_JoltWorld(joltWorld) {}
        ~JoltWorldHelper() = default;

        void Initialize(JPH::PhysicsSystem& physics_system);
        void StepWorld(Timestep deltaTime, JPH::PhysicsSystem& physics_system);

    private:
        JoltWorld* m_JoltWorld = nullptr;
        
        std::unique_ptr<JPH::TempAllocatorImpl>   m_TempAllocator;
        std::unique_ptr<JPH::JobSystemThreadPool> m_JobSystem;

        BPLayerInterfaceImpl               m_BPLayerInterface;
        ObjectVsBroadPhaseLayerFilterImpl  m_ObjectVsBroadPhaseLayerFilter;
        ObjectLayerPairFilterImpl          m_ObjectLayerPairFilter;
        MyBodyActivationListener           m_BodyActivationListener;

        bool m_Initialized = false;
    };
}
