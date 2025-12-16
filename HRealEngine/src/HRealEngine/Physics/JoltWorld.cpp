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
        m_JoltWorldHelper->Initialize(physics_system);
        // The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
        // variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
        body_interface = &physics_system.GetBodyInterface();

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
                auto layer = Layers::NON_MOVING;
                switch (rb3d.Type)
                {
                case Rigidbody3DComponent::BodyType::Static:
                    motionType = JPH::EMotionType::Static;
                    layer = Layers::NON_MOVING;
                    break;
                case Rigidbody3DComponent::BodyType::Dynamic:
                    motionType = JPH::EMotionType::Dynamic;
                    layer = Layers::MOVING;
                    break;
                case Rigidbody3DComponent::BodyType::Kinematic:
                    motionType = JPH::EMotionType::Kinematic;
                    layer = Layers::MOVING;
                    break;
                }
                glm::quat q = glm::quat(transform.Rotation); // (pitch/yaw/roll) rad
                JPH::Quat joltRot(q.x, q.y, q.z, q.w);

                JPH::BodyCreationSettings bodySettings(boxShape, JPH::RVec3(transform.Position.x, transform.Position.y, transform.Position.z),
                    joltRot, motionType, layer);

                JPH::Body* body = body_interface->CreateBody(bodySettings); // Note that if we run out of bodies this can return nullptr
                body_interface->AddBody(body->GetID(), JPH::EActivation::Activate);
                rb3d.RuntimeBody = body;
            }
        }

        // Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
        // You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
        // Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
        physics_system.OptimizeBroadPhase();
    }

    void JoltWorld::DestroyEntityPhysics(Entity entity)
    {
        if (entity.HasComponent<Rigidbody3DComponent>())
        {
            auto& rb = entity.GetComponent<Rigidbody3DComponent>();
            if (rb.RuntimeBody)
            {
                auto body = (JPH::Body*)rb.RuntimeBody;
                body_interface->RemoveBody(body->GetID());
                body_interface->DestroyBody(body->GetID());
                rb.RuntimeBody = nullptr;
            }
        }
    }

    void JoltWorld::Step3DWorld(Timestep deltaTime)
    {
        auto view = m_Scene->GetRegistry().view<Rigidbody3DComponent>();
        for (auto e : view)
        {
            Entity entity = { e, m_Scene };
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& rb3d = entity.GetComponent<Rigidbody3DComponent>();

            auto body = (JPH::Body*)rb3d.RuntimeBody;
            JPH::RVec3 position;
            JPH::Quat rotation;
            
            body_interface->GetPositionAndRotation(body->GetID(), position, rotation);
            transform.Position.x = position.GetX();
            transform.Position.y = position.GetY();
            transform.Position.z = position.GetZ();
            
            glm::quat q(rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ());
            glm::vec3 euler = glm::eulerAngles(q);
            transform.Rotation.x = euler.x;
            transform.Rotation.y = euler.y;
            transform.Rotation.z = euler.z;
        }
        m_JoltWorldHelper->StepWorld(deltaTime, physics_system);
    }

    void JoltWorld::UpdateSimulation3D(Timestep deltaTime, int& stepFrames)
    {
        if (!m_Scene->IsPaused() || stepFrames-- > 0)
        {
            auto view = m_Scene->GetRegistry().view<Rigidbody3DComponent>();
            for (auto e : view)
            {
                Entity entity = { e, m_Scene };
                auto& transform = entity.GetComponent<TransformComponent>();
                auto& rb3d = entity.GetComponent<Rigidbody3DComponent>();

                auto body = (JPH::Body*)rb3d.RuntimeBody;
                JPH::RVec3 position;
                JPH::Quat rotation;
            
                body_interface->GetPositionAndRotation(body->GetID(), position, rotation);
                transform.Position.x = position.GetX();
                transform.Position.y = position.GetY();
                transform.Position.z = position.GetZ();
            
                glm::quat q(rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ());
                glm::vec3 euler = glm::eulerAngles(q);
                transform.Rotation.x = euler.x;
                transform.Rotation.y = euler.y;
                transform.Rotation.z = euler.z;
            }
            m_JoltWorldHelper->StepWorld(deltaTime, physics_system);
        }
    }
}
