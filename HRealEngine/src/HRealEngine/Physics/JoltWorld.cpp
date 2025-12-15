#include "HRpch.h"
#include "JoltWorld.h"

#include "HRealEngine/Core/Components.h"
#include "HRealEngine/Core/Entity.h"
#include "HRealEngine/Scene/Scene.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/Body/BodyCreationSettings.h"
#include "Physics/Collision/Shape/BoxShape.h"

namespace HRealEngine
{
    JoltWorld::JoltWorld(Scene* scene) : m_Scene(scene)
    {
        m_JoltWorldHelper = CreateScope<JoltWorldHelper>(this);
    }

    JoltWorld::~JoltWorld()
    {
        
    }

    void JoltWorld::Init()
    {
        JPH::PhysicsSystem physics_system;
        m_JoltWorldHelper->Initialize(physics_system);
        // The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
        // variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
        JPH::BodyInterface &body_interface = physics_system.GetBodyInterface();

        auto view = m_Scene->GetRegistry().view<Rigidbody3DComponent>();
        for (auto e : view)
        {
            Entity entity = {e, m_Scene};
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& rb3d = entity.GetComponent<Rigidbody3DComponent>();

            if (rb3d.Shape == Rigidbody3DComponent::CollisionShape::Box)
            {
                JPH::BoxShapeSettings boxShapeSettings({ transform.Scale.x * 0.5f, transform.Scale.y * 0.5f, transform.Scale.z * 0.5f });
                boxShapeSettings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.

                JPH::ShapeSettings::ShapeResult boxShapeResult = boxShapeSettings.Create();
                JPH::ShapeRefC boxShape = boxShapeResult.Get(); // We don't expect an error here, but you can check boxShapeResult for HasError() / GetError()

                JPH::EMotionType motionType = JPH::EMotionType::Static;
                switch (rb3d.Type)
                {
                case Rigidbody3DComponent::BodyType::Static:
                    motionType = JPH::EMotionType::Static;
                    break;
                case Rigidbody3DComponent::BodyType::Dynamic:
                    motionType = JPH::EMotionType::Dynamic;
                    break;
                case Rigidbody3DComponent::BodyType::Kinematic:
                    motionType = JPH::EMotionType::Kinematic;
                    break;
                }
                glm::quat q = glm::quat(transform.Rotation); // (pitch/yaw/roll) rad
                JPH::Quat joltRot(q.x, q.y, q.z, q.w);

                JPH::BodyCreationSettings bodySettings(boxShape, JPH::RVec3(transform.Position.x, transform.Position.y, transform.Position.z),
                    joltRot, motionType, Layers::NON_MOVING);

                JPH::Body* body = body_interface.CreateBody(bodySettings); // Note that if we run out of bodies this can return nullptr
                body_interface.AddBody(body->GetID(), JPH::EActivation::Activate);
            }
        }

        // Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
        // You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
        // Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
        physics_system.OptimizeBroadPhase();
    }
}
