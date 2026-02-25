#include "HRpch.h"
#include "JoltWorld.h"

#include "HRealEngine/Core/Components.h"
#include "HRealEngine/Core/Entity.h"
#include "HRealEngine/Scene/Scene.h"
#include "HRealEngine/Scene/ScriptableEntity.h"
#include "HRealEngine/Scripting/ScriptEngine.h"
#include "HRealEngine/Utils/PlatformUtils.h"

#include "Physics/PhysicsSystem.h"
#include "Physics/Body/BodyCreationSettings.h"
#include "Physics/Collision/CastResult.h"
#include "Physics/Collision/CollisionCollectorImpl.h"
#include "Physics/Collision/RayCast.h"
#include "Physics/Collision/Shape/BoxShape.h"
#include "Physics/Collision/Shape/SphereShape.h"
#include "Physics/Collision/Shape/EmptyShape.h"
#include "Physics/Collision/Shape/RotatedTranslatedShape.h"

namespace HRealEngine
{
    static JPH::EAllowedDOFs GetAllowedDOFs(const Rigidbody3DComponent& rb)
    {
        JPH::EAllowedDOFs dofs = JPH::EAllowedDOFs(0);

        if (!rb.lockPositionX) 
            dofs |= JPH::EAllowedDOFs::TranslationX;
        if (!rb.lockPositionY) 
            dofs |= JPH::EAllowedDOFs::TranslationY;
        if (!rb.lockPositionZ) 
            dofs |= JPH::EAllowedDOFs::TranslationZ;
        if (!rb.lockRotationX) 
            dofs |= JPH::EAllowedDOFs::RotationX;
        if (!rb.lockRotationY) 
            dofs |= JPH::EAllowedDOFs::RotationY;
        if (!rb.lockRotationZ) 
            dofs |= JPH::EAllowedDOFs::RotationZ;

        return dofs;
    }
    
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
        CreatePercaptionBodies();
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
                bodySettings.mAllowedDOFs = GetAllowedDOFs(rb3d);

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
                if (boxCollider.bIsTrigger)
                    body->SetIsSensor(true);
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
            bodySettings.mAllowedDOFs = GetAllowedDOFs(rb3d);
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
                bodySettings.mAllowedDOFs = GetAllowedDOFs(rb3d);

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
                if (boxCollider.bIsTrigger)
                    body->SetIsSensor(true);
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
            bodySettings.mAllowedDOFs = GetAllowedDOFs(rb3d);
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
        std::vector<CollisionEvent> beginEvents, endEvents;
        std::vector<PerceptionOverlapEvent> perceptionEvents;
        {
            std::lock_guard<std::mutex> lock(m_EventQueueMutex);
            beginEvents = std::move(m_CollisionBeginEvents);
            endEvents = std::move(m_CollisionEndEvents);
            perceptionEvents = std::move(m_PerceptionOverlapEvents);
            
            m_CollisionBeginEvents.clear();
            m_CollisionEndEvents.clear();
            m_PerceptionOverlapEvents.clear();
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
                    if (nsc.Instance) 
                        nsc.Instance->OnCollisionBegin(b);
                }
                if (b.HasComponent<NativeScriptComponent>())
                    {
                    auto& nsc = b.GetComponent<NativeScriptComponent>();
                    if (nsc.Instance) 
                        nsc.Instance->OnCollisionBegin(a);
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
                    if (nsc.Instance)
                        nsc.Instance->OnCollisionEnd(b);
                }
                if (b.HasComponent<NativeScriptComponent>())
                {
                    auto& nsc = b.GetComponent<NativeScriptComponent>();
                    if (nsc.Instance)
                        nsc.Instance->OnCollisionEnd(a);
                }
            }
        }
        for (const auto& ev : perceptionEvents)
        {
            Entity perceiver = m_Scene->GetEntityByUUID(ev.EntityA);
            if (!perceiver || !perceiver.HasComponent<AIControllerComponent>())
                continue;

            auto& ai = perceiver.GetComponent<AIControllerComponent>();

            if (ev.bIsBegin)
            {
                if (ev.EntityB == ai.OwnerEntityID)
                    continue;
                ai.OverlappingEntities.insert(ev.EntityB);
                LOG_CORE_WARN("PERCEPTION OVERLAP BEGIN: Perceiver {} detected entity {}", (uint32_t)ev.EntityA, (uint32_t)ev.EntityB);
            }
            else
            {
                ai.OverlappingEntities.erase(ev.EntityB);
                LOG_CORE_WARN("PERCEPTION OVERLAP END: Perceiver {} lost entity {}", (uint32_t)ev.EntityA, (uint32_t)ev.EntityB);
            }
        }
        UpdatePercaptionBodies();
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
        if (entity.HasComponent<AIControllerComponent>())
        {
            auto& ai = entity.GetComponent<AIControllerComponent>();
            if (ai.SightRuntimeBody)
            {
                JPH::Body* body = (JPH::Body*)ai.SightRuntimeBody;
                body->SetUserData(0);
                body_interface->RemoveBody(body->GetID());
                body_interface->DestroyBody(body->GetID());
                ai.SightRuntimeBody = nullptr;
            }
            if (ai.HearingRuntimeBody)
            {
                JPH::Body* body = (JPH::Body*)ai.HearingRuntimeBody;
                body->SetUserData(0);
                body_interface->RemoveBody(body->GetID());
                body_interface->DestroyBody(body->GetID());
                ai.HearingRuntimeBody = nullptr;
            }
        }
        if (entity.HasComponent<PerceivableComponent>())
        {
            auto& perc = entity.GetComponent<PerceivableComponent>();
            if (perc.RuntimeBody)
            {
                JPH::Body* body = (JPH::Body*)perc.RuntimeBody;
                body->SetUserData(0);
                body_interface->RemoveBody(body->GetID());
                body_interface->DestroyBody(body->GetID());
                perc.RuntimeBody = nullptr;
            }
        }
    }

    void JoltWorld::Stop3DPhysics()
    {
        DestroyPercaptionBodies();
        ScriptEngine::SetBodyInterface(nullptr);
        m_JoltWorldHelper = nullptr;
    }

    RaycastHit3D JoltWorld::Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, bool debugDraw, float debugLifetime, const std::vector<uint64_t>& ignoreEntities)
    {
        RaycastHit3D result;
        glm::vec3 dir = glm::normalize(direction);
        
        JPH::RRayCast ray;
        ray.mOrigin = JPH::RVec3(origin.x, origin.y, origin.z);
        ray.mDirection = JPH::Vec3(dir.x, dir.y, dir.z) * maxDistance;

        if (ignoreEntities.empty())
        {
            JPH::RayCastResult hit;
        
            if (physics_system.GetNarrowPhaseQuery().CastRay(ray, hit))
            {
                result.Hit = true;
                result.Distance = hit.mFraction * maxDistance;
                result.HitPoint = origin + dir * result.Distance;
            
                JPH::BodyLockRead lock(physics_system.GetBodyLockInterface(), hit.mBodyID);
                if (lock.Succeeded())
                {
                    const JPH::Body& body = lock.GetBody();
                    result.HitEntityID = (UUID)body.GetUserData();

                    JPH::Vec3 normal = body.GetWorldSpaceSurfaceNormal(hit.mSubShapeID2,ray.GetPointOnRay(hit.mFraction));
                    result.HitNormal = glm::vec3(normal.GetX(), normal.GetY(), normal.GetZ());
                }
            }
        }
        else
        {
            JPH::AllHitCollisionCollector<JPH::CastRayCollector> collector;
            JPH::RayCastSettings settings;
            physics_system.GetNarrowPhaseQuery().CastRay(ray, settings, collector);
            collector.Sort();

            for (auto& hit : collector.mHits)
            {
                JPH::BodyLockRead lock(physics_system.GetBodyLockInterface(), hit.mBodyID);
                if (!lock.Succeeded())
                    continue;

                const JPH::Body& body = lock.GetBody();
                uint64_t entityID = body.GetUserData();
                
                bool shouldIgnore = false;
                for (uint64_t ignoreID : ignoreEntities)
                    if (entityID == ignoreID)
                    {
                        shouldIgnore = true;
                        break;
                    }
                if (shouldIgnore)
                    continue;

                result.Hit = true;
                result.Distance = hit.mFraction * maxDistance;
                result.HitPoint = origin + dir * result.Distance;
                result.HitEntityID = (UUID)entityID;
                JPH::Vec3 normal = body.GetWorldSpaceSurfaceNormal(hit.mSubShapeID2, ray.GetPointOnRay(hit.mFraction));
                result.HitNormal = glm::vec3(normal.GetX(), normal.GetY(), normal.GetZ());
                break;
            }
        }
        
        if (debugDraw)
        {
            glm::vec3 endPoint = origin + dir * maxDistance;
            if (result.Hit)
            {
                m_DebugLines.push_back({ origin, result.HitPoint, {0, 1, 0, 1}, debugLifetime });
                m_DebugLines.push_back({ result.HitPoint, endPoint, {1, 0, 0, 0.4f}, debugLifetime });
                m_DebugLines.push_back({ result.HitPoint, result.HitPoint + result.HitNormal * 0.5f, {0, 0, 1, 1}, debugLifetime });
            }
            else
            {
                m_DebugLines.push_back({ origin, endPoint, {1, 0, 0, 1}, debugLifetime });
            }
        }
        return result;
    }

    std::vector<RaycastHit3D> JoltWorld::RaycastAll(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, bool debugDraw, float debugLifetime, const std::vector<uint64_t>& ignoreEntities)
    {
        std::vector<RaycastHit3D> results;
        glm::vec3 dir = glm::normalize(direction);

        JPH::RRayCast ray;
        ray.mOrigin = JPH::RVec3(origin.x, origin.y, origin.z);
        ray.mDirection = JPH::Vec3(dir.x, dir.y, dir.z) * maxDistance;
    
        JPH::AllHitCollisionCollector<JPH::CastRayCollector> collector;
        JPH::RayCastSettings settings;
        physics_system.GetNarrowPhaseQuery().CastRay(ray, settings, collector);
        collector.Sort();

        for (auto& hit : collector.mHits)
        {
            JPH::BodyLockRead lock(physics_system.GetBodyLockInterface(), hit.mBodyID);
            if (!lock.Succeeded())
                continue;

            const JPH::Body& body = lock.GetBody();
            uint64_t entityID = body.GetUserData();
            
            bool shouldIgnore = false;
            for (uint64_t ignoreID : ignoreEntities)
                if (entityID == ignoreID)
                {
                    shouldIgnore = true;
                    break;
                }
            if (shouldIgnore)
                continue;

            RaycastHit3D r;
            r.Hit = true;
            r.Distance = hit.mFraction * maxDistance;
            r.HitPoint = origin + dir * r.Distance;
            r.HitEntityID = (UUID)entityID;
            JPH::Vec3 normal = body.GetWorldSpaceSurfaceNormal(hit.mSubShapeID2, ray.GetPointOnRay(hit.mFraction));
            r.HitNormal = glm::vec3(normal.GetX(), normal.GetY(), normal.GetZ());
            results.push_back(r);
        }

        if (debugDraw)
        {
            if (results.empty())
                m_DebugLines.push_back({ origin, origin + dir * maxDistance, {1, 0, 0, 1}, debugLifetime });
            else
            {
                m_DebugLines.push_back({ origin, results.front().HitPoint, {0, 1, 0, 1}, debugLifetime });
                for (auto& r : results)
                    m_DebugLines.push_back({ r.HitPoint, r.HitPoint + r.HitNormal * 0.5f, {0, 0, 1, 1}, debugLifetime });
                m_DebugLines.push_back({ results.back().HitPoint, origin + dir * maxDistance, {1, 0, 0, 0.4f}, debugLifetime });
            }
        }
        return results;
    }

    void JoltWorld::UpdateDebugLines(float deltaTime)
    {
        for (auto& line : m_DebugLines)
            line.RemainingLifetime -= deltaTime;

        m_DebugLines.erase(
            std::remove_if(m_DebugLines.begin(), m_DebugLines.end(),
                [](const DebugLine& l) { return l.RemainingLifetime < 0.0f; }),
            m_DebugLines.end()
        );
    }

    void JoltWorld::CreatePercaptionBodies()
    {
        auto viewAI = m_Scene->GetRegistry().view<AIControllerComponent, TransformComponent>();
        for (auto e : viewAI)
        {
            Entity entity{ e, m_Scene };
            auto& ai = entity.GetComponent<AIControllerComponent>();
            auto& tc = entity.GetComponent<TransformComponent>();
            
            if (ai.IsSightEnabled() && ai.SightSettings.SightRadius > 0.01f)
            {
                JPH::SphereShapeSettings sphereSettings(ai.SightSettings.SightRadius);
                sphereSettings.SetEmbedded();
                JPH::ShapeRefC shape = sphereSettings.Create().Get();

                JPH::BodyCreationSettings bodySettings(shape, JPH::RVec3(tc.Position.x, tc.Position.y, tc.Position.z), JPH::Quat::sIdentity(),
                    JPH::EMotionType::Kinematic, Layers::PERCEPTION);
                bodySettings.mIsSensor = true;
                bodySettings.mAllowSleeping = false;

                JPH::Body* body = body_interface->CreateBody(bodySettings);
                if (body)
                {
                    body->SetUserData(entity.GetUUID());
                    body_interface->AddBody(body->GetID(), JPH::EActivation::Activate);
                    ai.SightRuntimeBody = body;
                    ai.OwnerEntityID = entity.GetUUID();
                    LOG_CORE_INFO("Perception: Sight body created for entity UUID {} (radius: {})",(uint32_t)entity.GetUUID(), ai.SightSettings.SightRadius);
                }
            }
            
            if (ai.IsHearingEnabled() && ai.HearingSettings.HearingRadius > 0.01f)
            {
                JPH::SphereShapeSettings sphereSettings(ai.HearingSettings.HearingRadius);
                sphereSettings.SetEmbedded();
                JPH::ShapeRefC shape = sphereSettings.Create().Get();

                JPH::BodyCreationSettings bodySettings(shape, JPH::RVec3(tc.Position.x, tc.Position.y, tc.Position.z), JPH::Quat::sIdentity(), 
                    JPH::EMotionType::Kinematic, Layers::PERCEPTION);
                bodySettings.mIsSensor = true;
                bodySettings.mAllowSleeping = false;

                JPH::Body* body = body_interface->CreateBody(bodySettings);
                if (body)
                {
                    body->SetUserData(entity.GetUUID());
                    body_interface->AddBody(body->GetID(), JPH::EActivation::Activate);
                    ai.HearingRuntimeBody = body;
                    ai.OwnerEntityID = entity.GetUUID();
                    LOG_CORE_INFO("Perception: Hearing body created for entity UUID {} (radius: {})",(uint32_t)entity.GetUUID(), ai.HearingSettings.HearingRadius);
                }
            }
        }
        
        auto viewPerc = m_Scene->GetRegistry().view<PerceivableComponent, TransformComponent>();
        for (auto e : viewPerc)
        {
            Entity entity{ e, m_Scene };
            auto& perc = entity.GetComponent<PerceivableComponent>();
            auto& tc = entity.GetComponent<TransformComponent>();

            if (!perc.bIsDetectable)
                continue;

            JPH::SphereShapeSettings sphereSettings(0.1f);
            sphereSettings.SetEmbedded();
            JPH::ShapeRefC shape = sphereSettings.Create().Get();

            JPH::BodyCreationSettings bodySettings(shape, JPH::RVec3(tc.Position.x, tc.Position.y, tc.Position.z), JPH::Quat::sIdentity(),
                JPH::EMotionType::Kinematic, Layers::PERCEIVABLE);
            bodySettings.mIsSensor = true;
            bodySettings.mAllowSleeping = false;

            JPH::Body* body = body_interface->CreateBody(bodySettings);
            if (body)
            {
                body->SetUserData(entity.GetUUID());
                body_interface->AddBody(body->GetID(), JPH::EActivation::Activate);
                perc.RuntimeBody = body;
                perc.OwnerEntityID = entity.GetUUID();
                LOG_CORE_INFO("Perception: Perceivable body created for entity UUID {}", (uint32_t)entity.GetUUID());
            }
        }
    }

    void JoltWorld::DestroyPercaptionBodies()
    {
        auto viewAI = m_Scene->GetRegistry().view<AIControllerComponent>();
        for (auto e : viewAI)
        {
            auto& ai = m_Scene->GetRegistry().get<AIControllerComponent>(e);
            if (ai.SightRuntimeBody)
            {
                JPH::Body* body = (JPH::Body*)ai.SightRuntimeBody;
                body->SetUserData(0);
                body_interface->RemoveBody(body->GetID());
                body_interface->DestroyBody(body->GetID());
                ai.SightRuntimeBody = nullptr;
            }
            if (ai.HearingRuntimeBody)
            {
                JPH::Body* body = (JPH::Body*)ai.HearingRuntimeBody;
                body->SetUserData(0);
                body_interface->RemoveBody(body->GetID());
                body_interface->DestroyBody(body->GetID());
                ai.HearingRuntimeBody = nullptr;
            }
        }
        
        auto viewPerc = m_Scene->GetRegistry().view<PerceivableComponent>();
        for (auto e : viewPerc)
        {
            auto& perc = m_Scene->GetRegistry().get<PerceivableComponent>(e);
            if (perc.RuntimeBody)
            {
                JPH::Body* body = (JPH::Body*)perc.RuntimeBody;
                body->SetUserData(0);
                body_interface->RemoveBody(body->GetID());
                body_interface->DestroyBody(body->GetID());
                perc.RuntimeBody = nullptr;
            }
        }
    }

    void JoltWorld::UpdatePercaptionBodies()
    {
        UpdateAIControllerBodies();
        UpdatePerceivableBodies();
        //m_PendingNoiseEvents.clear();
        float now = Time::GetTime();
        float maxInterval = m_MaxIntervalForNoiseEvent;
        m_PendingNoiseEvents.erase(std::remove_if(m_PendingNoiseEvents.begin(), m_PendingNoiseEvents.end(),[now, maxInterval](const NoiseEvent& n)
        {
            return now - n.Timestamp > maxInterval;
        }),m_PendingNoiseEvents.end());
    }

    void JoltWorld::UpdateAIControllerBodies()
    {
        m_MaxIntervalForNoiseEvent = 0.5f;
        auto viewAI = m_Scene->GetRegistry().view<AIControllerComponent, TransformComponent>();
        for (auto e : viewAI)
        {
            auto& ai = m_Scene->GetRegistry().get<AIControllerComponent>(e);
            if (ai.UpdateInterval > m_MaxIntervalForNoiseEvent)
                m_MaxIntervalForNoiseEvent = ai.UpdateInterval;
            
            auto& tc = m_Scene->GetRegistry().get<TransformComponent>(e);
            
            JPH::RVec3 pos(tc.Position.x, tc.Position.y, tc.Position.z);
            glm::quat q = glm::quat(tc.Rotation);
            JPH::Quat rot(q.x, q.y, q.z, q.w);

            if (ai.SightRuntimeBody)
            {
                JPH::Body* body = (JPH::Body*)ai.SightRuntimeBody;
                body_interface->SetPositionAndRotation(body->GetID(), pos, rot, JPH::EActivation::Activate);
            }
            if (ai.HearingRuntimeBody)
            {
                JPH::Body* body = (JPH::Body*)ai.HearingRuntimeBody;
                body_interface->SetPositionAndRotation(body->GetID(), pos, rot, JPH::EActivation::Activate);
            }
            
            ai.TimeSinceLastUpdate += Time::GetDeltaTime();
            if (ai.TimeSinceLastUpdate < ai.UpdateInterval)
                continue;
            ai.TimeSinceLastUpdate = 0.0f;

            ai.PreviousPerceptions = ai.CurrentPerceptions;
            ai.CurrentPerceptions.clear();

            glm::quat ownerQuat = glm::quat(tc.Rotation);
            glm::vec3 forward = ownerQuat * glm::vec3(0, 0, -1);
            
            HearingPercaptionsUpdate(ai, tc);
            SightPercaptionsUpdate(ai, tc, forward);
            
            Entity perceiverEntity{ e, m_Scene };
            
            for (auto& curr : ai.CurrentPerceptions)
            {
                bool wasPerceived = false;
                for (auto& prev : ai.PreviousPerceptions)
                    if (prev.EntityID.ID == curr.EntityID.ID)
                    {
                        wasPerceived = true;
                        break;
                    }
                if (!wasPerceived)
                {
                    ai.ForgottenPerceptions.erase(std::remove_if(ai.ForgottenPerceptions.begin(), ai.ForgottenPerceptions.end(),[&](const PercaptionResult& f)
                    {
                        return f.EntityID.ID == curr.EntityID.ID;
                    }),ai.ForgottenPerceptions.end());
        
                    ScriptEngine::OnEntityPerceived(perceiverEntity, curr.EntityID.ID, curr.PercaptionMethod, curr.SensedPosition);
                }
            }
            
            for (auto& prev : ai.PreviousPerceptions)
            {
                bool stillPerceived = false;
                for (auto& curr : ai.CurrentPerceptions)
                    if (curr.EntityID.ID == prev.EntityID.ID)
                    {
                        stillPerceived = true;
                        break;
                    }
                
                if (!stillPerceived)
                {
                    ScriptEngine::OnEntityLost(perceiverEntity, prev.EntityID.ID, prev.SensedPosition);
                    
                    bool alreadyForgotten = false;
                    for (auto& f : ai.ForgottenPerceptions)
                        if (f.EntityID.ID == prev.EntityID.ID)
                        {
                            f.SensedPosition = prev.SensedPosition;
                            f.TimeSinceLastSensed = 0.0f;
                            alreadyForgotten = true; 
                            break;
                        }
                    if (!alreadyForgotten)
                        ai.ForgottenPerceptions.push_back(prev);
                }
            }

            float forgetDuration = ai.SightSettings.ForgetDuration;
            for (auto& f : ai.ForgottenPerceptions)
            {
                f.TimeSinceLastSensed += ai.UpdateInterval;
                if (f.TimeSinceLastSensed >= forgetDuration)
                    ScriptEngine::OnEntityForgotten(perceiverEntity, f.EntityID.ID);
            }
            
            ai.ForgottenPerceptions.erase(std::remove_if(ai.ForgottenPerceptions.begin(), ai.ForgottenPerceptions.end(),[forgetDuration](const PercaptionResult& r)
            {
                return r.TimeSinceLastSensed >= forgetDuration;
            }),ai.ForgottenPerceptions.end());
        }
    }

    void JoltWorld::HearingPercaptionsUpdate(AIControllerComponent& ai, const TransformComponent& tc)
    {
        if (ai.IsHearingEnabled())
            for (const auto& noise : m_PendingNoiseEvents)
            {
                if (noise.Timestamp < (Time::GetTime() - ai.UpdateInterval))
                    continue;
                
                if (noise.SourceEntityID == ai.OwnerEntityID)
                    continue;

                if (ai.OverlappingEntities.find(noise.SourceEntityID) == ai.OverlappingEntities.end())
                    continue;
                
                bool typeMatch = ai.HearingSettings.DetectableTypes.empty();
                if (!typeMatch)
                    for (auto& dt : ai.HearingSettings.DetectableTypes)
                        if (dt == noise.SourceType)
                        {
                            typeMatch = true; 
                            break;
                        }
                
                if (!typeMatch)
                    continue;

                float dist = glm::length(noise.Position - tc.Position);
                float effectiveRange = noise.MaxRange > 0.0f ? noise.MaxRange : ai.HearingSettings.HearingRadius;
                if (dist > effectiveRange)
                    continue;

                float perceivedLoudness = noise.Loudness * (1.0f - dist / effectiveRange);
                if (perceivedLoudness < 0.05f)
                    continue;

                bool alreadyPerceived = false;
                for (auto& cp : ai.CurrentPerceptions)
                    if (cp.EntityID.ID == noise.SourceEntityID)
                    {
                        alreadyPerceived = true;
                        break;
                    }

                if (!alreadyPerceived)
                {
                    PercaptionResult result;
                    result.EntityID.ID = noise.SourceEntityID;
                    result.Type = noise.SourceType;
                    result.PercaptionMethod = PercaptionType::Hearing;
                    result.SensedPosition = noise.Position;
                    result.TimeSinceLastSensed = 0.0f;
                    ai.CurrentPerceptions.push_back(result);
                }
            }
    }

    void JoltWorld::SightPercaptionsUpdate(AIControllerComponent& ai, const TransformComponent& tc,
        const glm::vec3& forward)
    {
        for (const UUID& targetID : ai.OverlappingEntities)
        {
            Entity targetEntity = m_Scene->GetEntityByUUID(targetID);
            if (!targetEntity || !targetEntity.HasComponent<PerceivableComponent>())
                continue;       
            
            auto& percComp = targetEntity.GetComponent<PerceivableComponent>();
            if (!percComp.bIsDetectable)
                continue;       
            
            auto& targetTC = targetEntity.GetComponent<TransformComponent>();
            glm::vec3 toTarget = targetTC.Position - tc.Position;
            float distance = glm::length(toTarget);
            if (distance < 0.001f)
                continue;       
            
            glm::vec3 dirToTarget = toTarget / distance;        
            if (ai.IsSightEnabled() && distance <= ai.SightSettings.SightRadius)
            {
                bool typeMatch = ai.SightSettings.DetectableTypes.empty();
                for (auto& dt : ai.SightSettings.DetectableTypes)
                    for (auto& pt : percComp.Types)
                        if (dt == pt)
                        {
                            typeMatch = true;
                            break;
                        }
                
                if (!typeMatch)
                    continue;       
                
                float dotProduct = glm::dot(glm::normalize(forward), dirToTarget);
                float halfFOVcos = glm::cos(glm::radians(ai.SightSettings.FieldOfView * 0.5f));
                if (dotProduct < halfFOVcos)
                    continue;       
                
                std::vector<uint64_t> ignoreList = { (uint64_t)ai.OwnerEntityID };
                RaycastHit3D losHit = Raycast(tc.Position, dirToTarget, distance + 0.1f, false, 0.0f, ignoreList);      
                bool hasLOS = false;
                
                if (losHit.Hit && losHit.HitEntityID == targetID)
                    hasLOS = true;
                else if (!percComp.DetectablePointsOffsets.empty())
                    for (auto& offset : percComp.DetectablePointsOffsets)
                    {
                        glm::vec3 targetPoint = targetTC.Position + offset;
                        glm::vec3 dir = glm::normalize(targetPoint - tc.Position);
                        float dist = glm::length(targetPoint - tc.Position);
                        RaycastHit3D pointHit = Raycast(tc.Position, dir, dist + 0.1f, false, 0.0f, ignoreList);
                        if (pointHit.Hit && pointHit.HitEntityID == targetID)
                        {
                            hasLOS = true;
                            break;
                        }
                    }       
                
                if (hasLOS)
                {
                    PercaptionResult result;
                    result.EntityID.ID = targetID;
                    result.Type = percComp.Types.empty() ? PerceivableType::Neutral : percComp.Types[0];
                    result.PercaptionMethod = PercaptionType::Sight;
                    result.SensedPosition = targetTC.Position;
                    result.TimeSinceLastSensed = 0.0f;
                    ai.CurrentPerceptions.push_back(result);
                }
            }
        }
    }

    void JoltWorld::UpdatePerceivableBodies()
    {
        auto viewPerc = m_Scene->GetRegistry().view<PerceivableComponent, TransformComponent>();
        for (auto e : viewPerc)
        {
            auto& perc = m_Scene->GetRegistry().get<PerceivableComponent>(e);
            auto& tc = m_Scene->GetRegistry().get<TransformComponent>(e);
            if (perc.RuntimeBody)
            {
                JPH::RVec3 pos(tc.Position.x, tc.Position.y, tc.Position.z);
                JPH::Body* body = (JPH::Body*)perc.RuntimeBody;
                body_interface->SetPosition(body->GetID(), pos, JPH::EActivation::Activate);
            }
        }
    }

    void JoltWorld::ReportNoise(const NoiseEvent& event)
    {
        NoiseEvent ev = event;
        ev.Timestamp = Time::GetTime();
        m_PendingNoiseEvents.push_back(ev);
    }
}
