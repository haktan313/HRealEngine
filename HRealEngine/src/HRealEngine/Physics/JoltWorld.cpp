#include "HRpch.h"
#include "JoltWorld.h"

#include "HRealEngine/Core/Components.h"
#include "HRealEngine/Core/Entity.h"
#include "HRealEngine/Scene/Scene.h"
#include "HRealEngine/Scene/ScriptableEntity.h"
#include "HRealEngine/Scripting/ScriptEngine.h"

#include "Physics/PhysicsSystem.h"
#include "Physics/Body/BodyCreationSettings.h"
#include "Physics/Collision/Shape/BoxShape.h"
#include "Physics/Collision/Shape/EmptyShape.h"
#include "Physics/Collision/Shape/RotatedTranslatedShape.h"

namespace HRealEngine
{
    JoltWorld::JoltWorld(Scene* scene) : m_Scene(scene), m_ContactListener(scene, this)
    {
        m_JoltWorldHelper = CreateScope<JoltWorldHelper>(this);
    }

    JoltWorld::~JoltWorld()
    {
        Stop3DPhysics();
    }

    void JoltWorld::Init()
    {
        m_JoltWorldHelper->Initialize(physics_system);
        physics_system.SetContactListener(&m_ContactListener);
        // The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
        // variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
        body_interface = &physics_system.GetBodyInterface();
        ScriptEngine::SetBodyInterface(body_interface);

        CreatePhysicsBodies();
        
        // Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
        // You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
        // Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
        physics_system.OptimizeBroadPhase();
    }

    void JoltWorld::CreatePhysicsBodies()
    {
        CreateBoxCollider();
        CreateEmptyBody();
    }

    void JoltWorld::CreateBoxCollider()
    {
        auto view = m_Scene->GetRegistry().view<BoxCollider3DComponent>();
        for (auto e : view)
        {
            Entity entity = {e, m_Scene};
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& boxCollider = entity.GetComponent<BoxCollider3DComponent>();
            if (entity.HasComponent<Rigidbody3DComponent>())
            {
                auto& rb3d = entity.GetComponent<Rigidbody3DComponent>();

                glm::vec3 halfExtents = glm::abs(transform.Scale) * boxCollider.Size;
                JPH::BoxShapeSettings boxShapeSettings({ halfExtents.x, halfExtents.y, halfExtents.z });
                boxShapeSettings.mConvexRadius = rb3d.ConvexRadius;
                boxShapeSettings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.

                JPH::ShapeSettings::ShapeResult boxShapeResult = boxShapeSettings.Create();
                JPH::ShapeRefC boxShape = boxShapeResult.Get(); // We don't expect an error here, but you can check boxShapeResult for HasError() / GetError()
                glm::vec3 localOffset = glm::abs(transform.Scale) * boxCollider.Offset;
                if (glm::length(localOffset) > 0.0001f)
                    boxShape = new JPH::RotatedTranslatedShape(JPH::Vec3(localOffset.x, localOffset.y, localOffset.z), JPH::Quat::sIdentity(), boxShape);
                
                JPH::EMotionType motionType = JPH::EMotionType::Static;
                auto layer = Layers::NON_MOVING;
                bool bAllowSleep = false;
                switch (rb3d.Type)
                {
                case Rigidbody3DComponent::BodyType::Static:
                    motionType = JPH::EMotionType::Static;
                    layer = Layers::NON_MOVING;
                    bAllowSleep = true;
                    break;
                case Rigidbody3DComponent::BodyType::Dynamic:
                    motionType = JPH::EMotionType::Dynamic;
                    layer = Layers::MOVING;
                    bAllowSleep = false;
                    break;
                case Rigidbody3DComponent::BodyType::Kinematic:
                    motionType = JPH::EMotionType::Kinematic;
                    layer = Layers::MOVING;
                    bAllowSleep = false;
                    break;
                }
                glm::quat q = glm::quat(transform.Rotation); // (pitch/yaw/roll) rad
                JPH::Quat joltRot(q.x, q.y, q.z, q.w);

                JPH::BodyCreationSettings bodySettings(boxShape, JPH::RVec3(transform.Position.x, transform.Position.y, transform.Position.z),
                    joltRot, motionType, layer);

                bodySettings.mAllowSleeping = bAllowSleep;
                bodySettings.mFriction = rb3d.Friction;
                bodySettings.mRestitution = rb3d.Restitution;
                JPH::Body* body = body_interface->CreateBody(bodySettings); // Note that if we run out of bodies this can return nullptr
                body->SetUserData(entity.GetUUID());
                if (boxCollider.bIsTrigger)
                    body->SetIsSensor(true);
                auto activation = motionType == JPH::EMotionType::Dynamic || motionType == JPH::EMotionType::Kinematic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate;
                body_interface->AddBody(body->GetID(), activation);

                rb3d.RuntimeBody = body;

                boxCollider.RuntimeBody = body;
            }
            else
            {
                glm::vec3 halfExtents = glm::abs(transform.Scale) * boxCollider.Size;
                JPH::BoxShapeSettings boxShapeSettings({ halfExtents.x, halfExtents.y, halfExtents.z });
                //boxShapeSettings.mConvexRadius = 0.0f;
                boxShapeSettings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.
                
                JPH::ShapeSettings::ShapeResult boxShapeResult = boxShapeSettings.Create();
                JPH::ShapeRefC boxShape = boxShapeResult.Get(); // We don't expect an error here, but you can check boxShapeResult for HasError() / GetError()
                
                glm::vec3 localOffset = glm::abs(transform.Scale) * boxCollider.Offset;
                if (glm::length(localOffset) > 0.0001f)
                    boxShape = new JPH::RotatedTranslatedShape(JPH::Vec3(localOffset.x, localOffset.y, localOffset.z), JPH::Quat::sIdentity(), boxShape);
                
                JPH::EMotionType motionType = JPH::EMotionType::Static;
                auto layer = Layers::NON_MOVING;
                bool bAllowSleep = true;
                
                glm::quat q = glm::quat(transform.Rotation); // (pitch/yaw/roll) rad
                JPH::Quat joltRot(q.x, q.y, q.z, q.w);
                
                JPH::BodyCreationSettings bodySettings(boxShape, JPH::RVec3(transform.Position.x, transform.Position.y, transform.Position.z),
                    joltRot, motionType, layer);
                
                bodySettings.mAllowSleeping = bAllowSleep;
                bodySettings.mFriction = 0.05f;
                bodySettings.mRestitution = 0.0f;
                bodySettings.mAllowDynamicOrKinematic = true; // Allow this body to be changed to dynamic or kinematic at runtime (by default only static bodies can be changed to dynamic/kinematic and not the other way around)
                JPH::Body* body = body_interface->CreateBody(bodySettings); // Note that if we run out of bodies this can return nullptr
                body->SetUserData(entity.GetUUID());
                
                auto activation = motionType == JPH::EMotionType::Dynamic || motionType == JPH::EMotionType::Kinematic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate;
                body_interface->AddBody(body->GetID(), activation);     
                
                boxCollider.RuntimeBody = body;
            }
        }
    }

    void JoltWorld::CreateEmptyBody()
    {
        auto viewRB = m_Scene->GetRegistry().view<Rigidbody3DComponent, TransformComponent>();
        for (auto e : viewRB)
        {
            Entity entity{ e, m_Scene };
            if (entity.HasComponent<BoxCollider3DComponent>())
                continue;
            
            auto& rb3d = entity.GetComponent<Rigidbody3DComponent>();
            auto& transform = entity.GetComponent<TransformComponent>();

            JPH::EmptyShapeSettings emptyShapeSettings;
            emptyShapeSettings.SetEmbedded();
            JPH::ShapeSettings::ShapeResult emptyShapeResult = emptyShapeSettings.Create();
            JPH::ShapeRefC emptyShape = emptyShapeResult.Get();
            LOG_CORE_WARN("Rigidbody3D without collider, emptyShapeBody created: Entity UUID {}", (uint32_t)entity.GetUUID());
            
            JPH::EMotionType motionType = JPH::EMotionType::Static;
            auto layer = Layers::NON_MOVING;
            bool bAllowSleep = false;
            switch (rb3d.Type)
            {
            case Rigidbody3DComponent::BodyType::Static:
                motionType = JPH::EMotionType::Static;
                layer = Layers::NON_MOVING;
                bAllowSleep = true;
                break;
            case Rigidbody3DComponent::BodyType::Dynamic:
                motionType = JPH::EMotionType::Dynamic;
                layer = Layers::MOVING;
                bAllowSleep = false;
                break;
            case Rigidbody3DComponent::BodyType::Kinematic:
                motionType = JPH::EMotionType::Kinematic;
                layer = Layers::MOVING;
                bAllowSleep = false;
                break;
            }
            
            glm::quat q = glm::quat(transform.Rotation);
            JPH::Quat joltRot(q.x, q.y, q.z, q.w);
            
            JPH::BodyCreationSettings bodySettings(emptyShape, JPH::RVec3(transform.Position.x, transform.Position.y, transform.Position.z),
                joltRot, motionType, layer);            
            bodySettings.mAllowSleeping = bAllowSleep;
            bodySettings.mAllowDynamicOrKinematic = true; // Allow this body to be changed to dynamic or kinematic at runtime (by default only static bodies can be changed to dynamic/kinematic and not the other way around)
            JPH::Body* body = body_interface->CreateBody(bodySettings);
            body->SetUserData(entity.GetUUID());
            auto activation = motionType == JPH::EMotionType::Dynamic || motionType == JPH::EMotionType::Kinematic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate;
            body_interface->AddBody(body->GetID(), activation);         
            rb3d.RuntimeBody = body;
        }
    }

    void JoltWorld::CreateBodyForEntity(Entity entity)
    {
        if (entity.HasComponent<BoxCollider3DComponent>())
        {
            auto& boxCollider = entity.GetComponent<BoxCollider3DComponent>();
            auto& transform = entity.GetComponent<TransformComponent>();
            
            if (entity.HasComponent<Rigidbody3DComponent>())
            {
                auto& rb3d = entity.GetComponent<Rigidbody3DComponent>();
                JPH::Vec3 savedVelocity = JPH::Vec3::sZero();
                if (rb3d.RuntimeBody)
                {
                    JPH::Body* oldBody = (JPH::Body*)rb3d.RuntimeBody;
                    savedVelocity = oldBody->GetLinearVelocity();
                    oldBody->SetUserData(0);
                    body_interface->RemoveBody(oldBody->GetID());
                    body_interface->DestroyBody(oldBody->GetID());
                    rb3d.RuntimeBody = nullptr;
                }
                if (boxCollider.RuntimeBody)
                {
                    JPH::Body* oldBody = (JPH::Body*)boxCollider.RuntimeBody;
                    oldBody->SetUserData(0);
                    body_interface->RemoveBody(oldBody->GetID());
                    body_interface->DestroyBody(oldBody->GetID());
                    boxCollider.RuntimeBody = nullptr;
                }
                
                glm::vec3 halfExtents = glm::abs(transform.Scale) * boxCollider.Size;
                JPH::BoxShapeSettings boxShapeSettings({ halfExtents.x, halfExtents.y, halfExtents.z });
                boxShapeSettings.mConvexRadius = rb3d.ConvexRadius;
                boxShapeSettings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.

                JPH::ShapeSettings::ShapeResult boxShapeResult = boxShapeSettings.Create();
                JPH::ShapeRefC boxShape = boxShapeResult.Get(); // We don't expect an error here, but you can check boxShapeResult for HasError() / GetError()
                glm::vec3 localOffset = glm::abs(transform.Scale) * boxCollider.Offset;
                if (glm::length(localOffset) > 0.0001f)
                    boxShape = new JPH::RotatedTranslatedShape(JPH::Vec3(localOffset.x, localOffset.y, localOffset.z), JPH::Quat::sIdentity(), boxShape);
                
                JPH::EMotionType motionType = JPH::EMotionType::Static;
                auto layer = Layers::NON_MOVING;
                bool bAllowSleep = false;
                switch (rb3d.Type)
                {
                case Rigidbody3DComponent::BodyType::Static:
                    motionType = JPH::EMotionType::Static;
                    layer = Layers::NON_MOVING;
                    bAllowSleep = true;
                    break;
                case Rigidbody3DComponent::BodyType::Dynamic:
                    motionType = JPH::EMotionType::Dynamic;
                    layer = Layers::MOVING;
                    bAllowSleep = false;
                    break;
                case Rigidbody3DComponent::BodyType::Kinematic:
                    motionType = JPH::EMotionType::Kinematic;
                    layer = Layers::MOVING;
                    bAllowSleep = false;
                    break;
                }
                glm::quat q = glm::quat(transform.Rotation); // (pitch/yaw/roll) rad
                JPH::Quat joltRot(q.x, q.y, q.z, q.w);

                JPH::BodyCreationSettings bodySettings(boxShape, JPH::RVec3(transform.Position.x, transform.Position.y, transform.Position.z),
                    joltRot, motionType, layer);

                bodySettings.mAllowSleeping = bAllowSleep;
                bodySettings.mFriction = rb3d.Friction;
                bodySettings.mRestitution = rb3d.Restitution;
                bodySettings.mAllowDynamicOrKinematic = true; // Allow this body to be changed to dynamic or kinematic at runtime (by default only static bodies can be changed to dynamic/kinematic and not the other way around)
                JPH::Body* body = body_interface->CreateBody(bodySettings); // Note that if we run out of bodies this can return nullptr
                body->SetUserData(entity.GetUUID());
                if (boxCollider.bIsTrigger)
                    body->SetIsSensor(true);
                auto activation = motionType == JPH::EMotionType::Dynamic || motionType == JPH::EMotionType::Kinematic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate;
                body_interface->AddBody(body->GetID(), activation);
                
                if (savedVelocity.LengthSq() > 0.0f)
                    body_interface->SetLinearVelocity(body->GetID(), savedVelocity);
                
                rb3d.RuntimeBody = body;
                boxCollider.RuntimeBody = body;
            }
            else
            {
                if (boxCollider.RuntimeBody)
                    return;

                glm::vec3 halfExtents = glm::abs(transform.Scale) * boxCollider.Size;
                JPH::BoxShapeSettings boxShapeSettings({ halfExtents.x, halfExtents.y, halfExtents.z });
                //boxShapeSettings.mConvexRadius = 0.0f;
                boxShapeSettings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.
                
                JPH::ShapeSettings::ShapeResult boxShapeResult = boxShapeSettings.Create();
                JPH::ShapeRefC boxShape = boxShapeResult.Get(); // We don't expect an error here, but you can check boxShapeResult for HasError() / GetError()
                
                glm::vec3 localOffset = glm::abs(transform.Scale) * boxCollider.Offset;
                if (glm::length(localOffset) > 0.0001f)
                    boxShape = new JPH::RotatedTranslatedShape(JPH::Vec3(localOffset.x, localOffset.y, localOffset.z), JPH::Quat::sIdentity(), boxShape);
                
                JPH::EMotionType motionType = JPH::EMotionType::Static;
                auto layer = Layers::NON_MOVING;
                bool bAllowSleep = true;
                
                glm::quat q = glm::quat(transform.Rotation); // (pitch/yaw/roll) rad
                JPH::Quat joltRot(q.x, q.y, q.z, q.w);
                
                JPH::BodyCreationSettings bodySettings(boxShape, JPH::RVec3(transform.Position.x, transform.Position.y, transform.Position.z),
                    joltRot, motionType, layer);
                
                bodySettings.mAllowSleeping = bAllowSleep;
                bodySettings.mFriction = 0.05f;
                bodySettings.mRestitution = 0.0f;
                bodySettings.mAllowDynamicOrKinematic = true; // Allow this body to be changed to dynamic or kinematic at runtime (by default only static bodies can be changed to dynamic/kinematic and not the other way around)
                JPH::Body* body = body_interface->CreateBody(bodySettings); // Note that if we run out of bodies this can return nullptr
                body->SetUserData(entity.GetUUID());
                
                auto activation = motionType == JPH::EMotionType::Dynamic || motionType == JPH::EMotionType::Kinematic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate;
                body_interface->AddBody(body->GetID(), activation);     
                
                boxCollider.RuntimeBody = body;
            }
        }
        else if (entity.HasComponent<Rigidbody3DComponent>())
        {
            auto& rb3d = entity.GetComponent<Rigidbody3DComponent>();
            auto& transform = entity.GetComponent<TransformComponent>();
            if (rb3d.RuntimeBody)
                return;
            JPH::EmptyShapeSettings emptyShapeSettings;
            emptyShapeSettings.SetEmbedded();
            JPH::ShapeSettings::ShapeResult emptyShapeResult = emptyShapeSettings.Create();
            JPH::ShapeRefC emptyShape = emptyShapeResult.Get();
            LOG_CORE_WARN("Rigidbody3D without collider, emptyShapeBody created: Entity UUID {}", (uint32_t)entity.GetUUID());
            
            JPH::EMotionType motionType = JPH::EMotionType::Static;
            auto layer = Layers::NON_MOVING;
            bool bAllowSleep = false;
            switch (rb3d.Type)
            {
            case Rigidbody3DComponent::BodyType::Static:
                motionType = JPH::EMotionType::Static;
                layer = Layers::NON_MOVING;
                bAllowSleep = true;
                break;
            case Rigidbody3DComponent::BodyType::Dynamic:
                motionType = JPH::EMotionType::Dynamic;
                layer = Layers::MOVING;
                bAllowSleep = false;
                break;
            case Rigidbody3DComponent::BodyType::Kinematic:
                motionType = JPH::EMotionType::Kinematic;
                layer = Layers::MOVING;
                bAllowSleep = false;
                break;
            }
            
            glm::quat q = glm::quat(transform.Rotation);
            JPH::Quat joltRot(q.x, q.y, q.z, q.w);
            
            JPH::BodyCreationSettings bodySettings(emptyShape, JPH::RVec3(transform.Position.x, transform.Position.y, transform.Position.z),
                joltRot, motionType, layer);            
            bodySettings.mAllowSleeping = bAllowSleep;
            bodySettings.mAllowDynamicOrKinematic = true; // Allow this body to be changed to dynamic or kinematic at runtime (by default only static bodies can be changed to dynamic/kinematic and not the other way around)
            JPH::Body* body = body_interface->CreateBody(bodySettings);
            body->SetUserData(entity.GetUUID());
            auto activation = motionType == JPH::EMotionType::Dynamic || motionType == JPH::EMotionType::Kinematic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate;
            body_interface->AddBody(body->GetID(), activation);         
            rb3d.RuntimeBody = body;
        }
    }

    void JoltWorld::SetBodyTypeForEntity(Entity entity)
    {
        if (!entity.HasComponent<Rigidbody3DComponent>())
            return;
        auto& rb = entity.GetComponent<Rigidbody3DComponent>();
        if (!rb.RuntimeBody)
        {
            LOG_CORE_ERROR("SetBodyTypeForEntity: RuntimeBody is null for entity with UUID {}", (uint32_t)entity.GetUUID());
            return;
        }
        auto body = (JPH::Body*)rb.RuntimeBody;
        if (!body)
        {
            LOG_CORE_ERROR("SetBodyTypeForEntity: Body is null for entity with UUID {}", (uint32_t)entity.GetUUID());
            return;
        }
        switch (rb.Type)
        {
        case Rigidbody3DComponent::BodyType::Static:
            body_interface->SetMotionType(body->GetID(), JPH::EMotionType::Static, JPH::EActivation::DontActivate);
            body_interface->SetObjectLayer(body->GetID(), Layers::NON_MOVING);
            break;
        case Rigidbody3DComponent::BodyType::Dynamic:
            body_interface->SetMotionType(body->GetID(), JPH::EMotionType::Dynamic, JPH::EActivation::Activate);
            body_interface->SetObjectLayer(body->GetID(), Layers::MOVING);
            break;
        case Rigidbody3DComponent::BodyType::Kinematic:
            body_interface->SetMotionType(body->GetID(), JPH::EMotionType::Kinematic, JPH::EActivation::Activate);
            body_interface->SetObjectLayer(body->GetID(), Layers::MOVING);
            break;
        default:
            break;
        }
    }

    void JoltWorld::SetIsTriggerForEntity(Entity entity, bool isTrigger)
    {
        auto& boxCollider = entity.GetComponent<BoxCollider3DComponent>();
        if (boxCollider.RuntimeBody)
        {
            auto body = (JPH::Body*)boxCollider.RuntimeBody;
            body->SetIsSensor(isTrigger);
        }
    }

    void JoltWorld::SetBoxColliderSizeForEntity(Entity entity, const glm::vec3& size)
    {
        if (!entity.HasComponent<BoxCollider3DComponent>())
            return;
        
        auto& boxCollider = entity.GetComponent<BoxCollider3DComponent>();
        boxCollider.Size = size;
        
        void* runtimeBody = entity.HasComponent<Rigidbody3DComponent>() ? entity.GetComponent<Rigidbody3DComponent>().RuntimeBody : boxCollider.RuntimeBody;
        
        if (!runtimeBody)
            return;
        
        auto& transform = entity.GetComponent<TransformComponent>();
        glm::vec3 halfExtents = glm::abs(transform.Scale) * size;
        
        JPH::BoxShapeSettings boxShapeSettings({ halfExtents.x, halfExtents.y, halfExtents.z });
        boxShapeSettings.SetEmbedded();
        
        glm::vec3 localOffset = glm::abs(transform.Scale) * boxCollider.Offset;
        JPH::ShapeRefC newShape = boxShapeSettings.Create().Get();
        if (glm::length(localOffset) > 0.0001f)
            newShape = new JPH::RotatedTranslatedShape(JPH::Vec3(localOffset.x, localOffset.y, localOffset.z), JPH::Quat::sIdentity(), newShape);
        
        JPH::Body* body = (JPH::Body*)runtimeBody;
        body_interface->SetShape(body->GetID(), newShape, true, JPH::EActivation::Activate);
    }

    void JoltWorld::SetBoxColliderOffsetForEntity(Entity entity, const glm::vec3& offset)
    {
        if (!entity.HasComponent<BoxCollider3DComponent>())
            return;
        
        auto& boxCollider = entity.GetComponent<BoxCollider3DComponent>();
        boxCollider.Offset = offset;
        
        void* runtimeBody = entity.HasComponent<Rigidbody3DComponent>() ? entity.GetComponent<Rigidbody3DComponent>().RuntimeBody : boxCollider.RuntimeBody;
        
        if (!runtimeBody)
            return;
        
        auto& transform = entity.GetComponent<TransformComponent>();
        glm::vec3 halfExtents = glm::abs(transform.Scale) * boxCollider.Size;
        
        JPH::BoxShapeSettings boxShapeSettings({ halfExtents.x, halfExtents.y, halfExtents.z });
        boxShapeSettings.SetEmbedded();
        
        JPH::ShapeRefC newShape = boxShapeSettings.Create().Get();
        glm::vec3 localOffset = glm::abs(transform.Scale) * offset;
        if (glm::length(localOffset) > 0.0001f)
            newShape = new JPH::RotatedTranslatedShape(JPH::Vec3(localOffset.x, localOffset.y, localOffset.z), JPH::Quat::sIdentity(), newShape);
        
        JPH::Body* body = (JPH::Body*)runtimeBody;
        body_interface->SetShape(body->GetID(), newShape, true, JPH::EActivation::Activate);
    }

    void JoltWorld::DestroyEntityPhysics(Entity entity)
    {
        if (entity.HasComponent<Rigidbody3DComponent>())
        {
            auto& rb = entity.GetComponent<Rigidbody3DComponent>();
            if (rb.RuntimeBody)
            {
                auto body = (JPH::Body*)rb.RuntimeBody;
                body->SetUserData(0);
                body_interface->RemoveBody(body->GetID());
                body_interface->DestroyBody(body->GetID());
                rb.RuntimeBody = nullptr;
            }
        }
    }

    void JoltWorld::Stop3DPhysics()
    {
        ScriptEngine::SetBodyInterface(nullptr);
        m_JoltWorldHelper = nullptr;
    }

    void JoltWorld::Step3DWorldForKinematicBodies(Timestep deltaTime)
    {
        auto view = m_Scene->GetRegistry().view<Rigidbody3DComponent, TransformComponent>();
        for (auto e : view)
        {
            Entity entity{ e, m_Scene };
            auto& rb = entity.GetComponent<Rigidbody3DComponent>();
            if (rb.Type != Rigidbody3DComponent::BodyType::Kinematic)
                continue;

            JPH::Body* body = (JPH::Body*)rb.RuntimeBody;
            if (!body)
                continue;

            auto& tc = entity.GetComponent<TransformComponent>();
                
            glm::quat q = glm::quat(tc.Rotation);
            JPH::Quat rot(q.x, q.y, q.z, q.w);
            JPH::RVec3 pos(tc.Position.x, tc.Position.y, tc.Position.z);

            body_interface->MoveKinematic(body->GetID(), pos, rot, deltaTime.GetSeconds());
        }
    }
    
    void JoltWorld::Step3DWorld(Timestep deltaTime)
    {
        Step3DWorldForKinematicBodies(deltaTime);
        m_JoltWorldHelper->StepWorld(deltaTime, physics_system);
        Step3DWorldForNonKinematicBodies();
    }

    void JoltWorld::Step3DWorldForNonKinematicBodies()
    {
        {
            auto view = m_Scene->GetRegistry().view<Rigidbody3DComponent>();
            for (auto e : view)
            {
                Entity entity = { e, m_Scene };
                auto& transform = entity.GetComponent<TransformComponent>();
                auto& rb3d = entity.GetComponent<Rigidbody3DComponent>();
                if (rb3d.Type == Rigidbody3DComponent::BodyType::Kinematic)
                    continue;

                auto body = (JPH::Body*)rb3d.RuntimeBody;
                JPH::RVec3 position;
                JPH::Quat rotation;
            
                body_interface->GetPositionAndRotation(body->GetID(), position, rotation);
                transform.Position.x = position.GetX();
                transform.Position.y = position.GetY();
                transform.Position.z = position.GetZ();
            
                glm::quat q;
                q.x = rotation.GetX();
                q.y = rotation.GetY();
                q.z = rotation.GetZ();
                q.w = rotation.GetW();
            
                glm::vec3 euler = glm::eulerAngles(q);
                transform.Rotation = euler;
            }
        }
        {
            auto view = m_Scene->GetRegistry().view<BoxCollider3DComponent, TransformComponent>();
            for (auto e : view)
            {
                Entity entity = { e, m_Scene };
                if (entity.HasComponent<Rigidbody3DComponent>())
                    continue;
                auto& transform = entity.GetComponent<TransformComponent>();
                auto& boxCollider = entity.GetComponent<BoxCollider3DComponent>();
                if (boxCollider.RuntimeBody)
                {
                    JPH::Body* body = (JPH::Body*)boxCollider.RuntimeBody;
                    JPH::RVec3 position;
                    JPH::Quat rotation;

                    body_interface->GetPositionAndRotation(body->GetID(), position, rotation);
                    transform.Position.x = position.GetX();
                    transform.Position.y = position.GetY();
                    transform.Position.z = position.GetZ();

                    glm::quat q;
                    q.x = rotation.GetX();
                    q.y = rotation.GetY();
                    q.z = rotation.GetZ();
                    q.w = rotation.GetW();

                    glm::vec3 euler = glm::eulerAngles(q);
                    transform.Rotation = euler;
                }
            }
        }
    }

    void JoltWorld::UpdateSimulation3DForKinematicBodies(Timestep deltaTime)
    {
        auto view = m_Scene->GetRegistry().view<Rigidbody3DComponent, TransformComponent>();
        for (auto e : view)
        {
            Entity entity{ e, m_Scene };
            auto& rb = entity.GetComponent<Rigidbody3DComponent>();
            if (rb.Type != Rigidbody3DComponent::BodyType::Kinematic)
                continue;

            JPH::Body* body = (JPH::Body*)rb.RuntimeBody;
            if (!body)
                continue;

            auto& tc = entity.GetComponent<TransformComponent>();
                
            glm::quat q = glm::quat(tc.Rotation);
            JPH::Quat rot(q.x, q.y, q.z, q.w);
            JPH::RVec3 pos(tc.Position.x, tc.Position.y, tc.Position.z);

            body_interface->MoveKinematic(body->GetID(), pos, rot, deltaTime.GetSeconds());
        }
    }

    void JoltWorld::UpdateSimulation3D(Timestep deltaTime, int& stepFrames)
    {
        if (!m_Scene->IsPaused() || stepFrames-- > 0)
        {
            UpdateSimulation3DForKinematicBodies(deltaTime);
            m_JoltWorldHelper->StepWorld(deltaTime, physics_system);
            UpdateSimulation3DForNonKinematicBodies();
        }
    }

    void JoltWorld::UpdateSimulation3DForNonKinematicBodies()
    {
        {
            auto view = m_Scene->GetRegistry().view<Rigidbody3DComponent>();
            for (auto e : view)
            {
                Entity entity = { e, m_Scene };
                auto& transform = entity.GetComponent<TransformComponent>();
                auto& rb3d = entity.GetComponent<Rigidbody3DComponent>();
                if (rb3d.Type == Rigidbody3DComponent::BodyType::Kinematic)
                    continue;           
                auto body = (JPH::Body*)rb3d.RuntimeBody;
                JPH::RVec3 position;
                JPH::Quat rotation;
            
                body_interface->GetPositionAndRotation(body->GetID(), position, rotation);
                transform.Position.x = position.GetX();
                transform.Position.y = position.GetY();
                transform.Position.z = position.GetZ();
            
                glm::quat q;
                q.x = rotation.GetX();
                q.y = rotation.GetY();
                q.z = rotation.GetZ();
                q.w = rotation.GetW();
            
                glm::vec3 euler = glm::eulerAngles(q);
                transform.Rotation = euler;
            }
        }
        {
            auto view = m_Scene->GetRegistry().view<BoxCollider3DComponent, TransformComponent>();
            for (auto e : view)
            {
                Entity entity = { e, m_Scene };
                if (entity.HasComponent<Rigidbody3DComponent>())
                    continue;
                auto& transform = entity.GetComponent<TransformComponent>();
                auto& boxCollider = entity.GetComponent<BoxCollider3DComponent>();
                if (boxCollider.RuntimeBody)
                {
                    JPH::Body* body = (JPH::Body*)boxCollider.RuntimeBody;
                    JPH::RVec3 position;
                    JPH::Quat rotation;
                        
                    body_interface->GetPositionAndRotation(body->GetID(), position, rotation);
                    transform.Position.x = position.GetX();
                    transform.Position.y = position.GetY();
                    transform.Position.z = position.GetZ();
                        
                    glm::quat q;
                    q.x = rotation.GetX();
                    q.y = rotation.GetY();
                    q.z = rotation.GetZ();
                    q.w = rotation.GetW();
                        
                    glm::vec3 euler = glm::eulerAngles(q);
                    transform.Rotation = euler;
                }
            }
        }
    }

    void JoltWorld::UpdateRuntime3D()
    {
        /*if (!m_CollisionBeginEvents.empty())
        {
            for (const auto& collisionEvent : m_CollisionBeginEvents)
            {
                if (m_Scene->GetRegistry().any_of<ScriptComponent>(collisionEvent.A))
                {
                    Entity entityA = {collisionEvent.A, m_Scene};
                    ScriptEngine::OnCollisionBegin(entityA, Entity{collisionEvent.B, m_Scene});
                }
                if (m_Scene->GetRegistry().any_of<ScriptComponent>(collisionEvent.B))
                {
                    Entity entityB = {collisionEvent.B, m_Scene};
                    ScriptEngine::OnCollisionBegin(entityB, Entity{collisionEvent.A, m_Scene});
                }       
                if (m_Scene->GetRegistry().any_of<NativeScriptComponent>(collisionEvent.A))
                {
                    Entity entityA = {collisionEvent.A, m_Scene};
                    auto& nsc = entityA.GetComponent<NativeScriptComponent>();
                    if (nsc.Instance)
                        nsc.Instance->OnCollisionBegin(Entity{collisionEvent.B, m_Scene});
                }
                if (m_Scene->GetRegistry().any_of<NativeScriptComponent>(collisionEvent.B))
                {
                    Entity entityB = {collisionEvent.B, m_Scene};
                    auto& nsc = entityB.GetComponent<NativeScriptComponent>();
                    if (nsc.Instance)
                        nsc.Instance->OnCollisionBegin(Entity{collisionEvent.A, m_Scene});
                }
            }
            m_CollisionBeginEvents.clear();
        }
        if (!m_CollisionEndEvents.empty())
        {
            for (const auto& collisionEvent : m_CollisionEndEvents)
            {
                if (m_Scene->GetRegistry().any_of<ScriptComponent>(collisionEvent.A))
                {
                    Entity entityA = {collisionEvent.A, m_Scene};
                    ScriptEngine::OnCollisionEnd(entityA, Entity{collisionEvent.B, m_Scene});
                }
                if (m_Scene->GetRegistry().any_of<ScriptComponent>(collisionEvent.B))
                {
                    Entity entityB = {collisionEvent.B, m_Scene};
                    ScriptEngine::OnCollisionEnd(entityB, Entity{collisionEvent.A, m_Scene});
                }       
                if (m_Scene->GetRegistry().any_of<NativeScriptComponent>(collisionEvent.A))
                {
                    Entity entityA = {collisionEvent.A, m_Scene};
                    auto& nsc = entityA.GetComponent<NativeScriptComponent>();
                    if (nsc.Instance)
                        nsc.Instance->OnCollisionEnd(Entity{collisionEvent.B, m_Scene});
                }
                if (m_Scene->GetRegistry().any_of<NativeScriptComponent>(collisionEvent.B))
                {
                    Entity entityB = {collisionEvent.B, m_Scene};
                    auto& nsc = entityB.GetComponent<NativeScriptComponent>();
                    if (nsc.Instance)
                        nsc.Instance->OnCollisionEnd(Entity{collisionEvent.A, m_Scene});
                }
            }
            m_CollisionEndEvents.clear();
        }*/
        std::vector<CollisionEvent> beginEvents, endEvents;
        {
            std::lock_guard<std::mutex> lock(m_EventQueueMutex);
            beginEvents = std::move(m_CollisionBeginEvents);
            endEvents = std::move(m_CollisionEndEvents);
        
            m_CollisionBeginEvents.clear();
            m_CollisionEndEvents.clear();
        }
        
        for (const auto& ev : beginEvents)
        {
            Entity a = m_Scene->GetEntityByUUID(ev.EntityA);
            Entity b = m_Scene->GetEntityByUUID(ev.EntityB);

            if (a && b)
            {
                if (m_Scene->GetRegistry().any_of<ScriptComponent>(a))
                    ScriptEngine::OnCollisionBegin(a, b);
            
                if (m_Scene->GetRegistry().any_of<ScriptComponent>(b))
                    ScriptEngine::OnCollisionBegin(b, a);
                
                if (a.HasComponent<NativeScriptComponent>())
                {
                    auto& nsc = a.GetComponent<NativeScriptComponent>();
                    if (nsc.Instance) nsc.Instance->OnCollisionBegin(b);
                }
                if (b.HasComponent<NativeScriptComponent>())
                    {
                    auto& nsc = b.GetComponent<NativeScriptComponent>();
                    if (nsc.Instance) nsc.Instance->OnCollisionBegin(a);
                }
            }
        }
        for (const auto& ev : endEvents)
        {
            Entity a = m_Scene->GetEntityByUUID(ev.EntityA);
            Entity b = m_Scene->GetEntityByUUID(ev.EntityB);
            if (a && b)
            {
                if (m_Scene->GetRegistry().any_of<ScriptComponent>(a))
                    ScriptEngine::OnCollisionEnd(a, b);
            
                if (m_Scene->GetRegistry().any_of<ScriptComponent>(b))
                    ScriptEngine::OnCollisionEnd(b, a);

                if (a.HasComponent<NativeScriptComponent>())
                    {
                    auto& nsc = a.GetComponent<NativeScriptComponent>();
                    if (nsc.Instance) nsc.Instance->OnCollisionEnd(b);
                }
                if (b.HasComponent<NativeScriptComponent>())
                    {
                    auto& nsc = b.GetComponent<NativeScriptComponent>();
                    if (nsc.Instance) nsc.Instance->OnCollisionEnd(a);
                }
            }
        }
    }
}
