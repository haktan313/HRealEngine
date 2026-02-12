#pragma once
#include "JoltWorldHelper.h"
#include "HRealEngine/Core/Entity.h"
#include <mutex>

namespace HRealEngine
{
    class Scene;
    class Timestep;
    
    class JoltWorld
    {
    public:
        JoltWorld(Scene* scene);
        ~JoltWorld();
        void Init();

        void CreatePhysicsBodies();
        void CreateBoxCollider();
        void CreateEmptyBody();

        void UpdateSimulation3DForKinematicBodies(Timestep deltaTime);
        void UpdateSimulation3D(Timestep deltaTime, int& stepFrames);
        void UpdateSimulation3DForNonKinematicBodies();
        void UpdateRuntime3D();
        void Step3DWorldForKinematicBodies(Timestep deltaTime);
        void Step3DWorld(Timestep deltaTime);
        void Step3DWorldForNonKinematicBodies();
        void DestroyEntityPhysics(Entity entity);
        void Stop3DPhysics();
        
    private:
        struct CollisionEvent
        {
            /*entt::entity A;
            entt::entity B;*/
            UUID EntityA;
            UUID EntityB;
        };
        class MyContactListener : public JPH::ContactListener
        {
        public:
            MyContactListener(Scene* scene, JoltWorld* joltWorld) : m_Scene(scene), m_JoltWorld(joltWorld) {}
            virtual JPH::ValidateResult	OnContactValidate(const JPH::Body &inBody1, const JPH::Body &inBody2,
                JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult &inCollisionResult) override
            {
                //std::cout << "Contact validate callback" << std::endl;
                // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
                return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
            }

            virtual void OnContactAdded(const JPH::Body &inBody1, const JPH::Body &inBody2,
                const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) override
            {
                /*auto entity1ID = (UUID)inBody1.GetUserData();
                auto entity2ID = (UUID)inBody2.GetUserData();
                Entity entity1 = m_Scene->GetEntityByUUID(entity1ID);
                Entity entity2 = m_Scene->GetEntityByUUID(entity2ID);
                if (m_Scene->GetRegistry().valid(entity1) && m_Scene->GetRegistry().valid(entity2))
                    m_JoltWorld->m_CollisionBeginEvents.push_back({ entity1,  entity2 });*/
                UUID entity1ID = (UUID)inBody1.GetUserData();
                UUID entity2ID = (UUID)inBody2.GetUserData();

                if (entity1ID != 0 && entity2ID != 0)
                {
                    std::lock_guard<std::mutex> lock(m_JoltWorld->m_EventQueueMutex);
                    m_JoltWorld->m_CollisionBeginEvents.push_back({ entity1ID, entity2ID });
                }
                std::cout << "A contact was added" << std::endl;
            }

            virtual void OnContactPersisted(const JPH::Body &inBody1, const JPH::Body &inBody2,
                const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) override
            {
                //std::cout << "A contact was persisted" << std::endl;
            }

            virtual void OnContactRemoved(const JPH::SubShapeIDPair &inSubShapePair) override
            {
                /*auto body1ID = inSubShapePair.GetBody1ID();
                auto body2ID = inSubShapePair.GetBody2ID();
                const JPH::BodyInterface &bi = m_JoltWorld->physics_system.GetBodyInterfaceNoLock();

                uint64_t userData1 = bi.GetUserData(body1ID);
                uint64_t userData2 = bi.GetUserData(body2ID);

                UUID entity1ID = (UUID)userData1;
                UUID entity2ID = (UUID)userData2;
                if (entity1ID == 0 || entity2ID == 0)
                    return;
                Entity entity1 = m_Scene->GetEntityByUUID(entity1ID);
                Entity entity2 = m_Scene->GetEntityByUUID(entity2ID);
                if (m_Scene->GetRegistry().valid(entity1) && m_Scene->GetRegistry().valid(entity2))
                    m_JoltWorld->m_CollisionEndEvents.push_back({ entity1,  entity2 });
                std::cout << "A contact was removed" << std::endl;*/
                const JPH::BodyInterface &bi = m_JoltWorld->physics_system.GetBodyInterfaceNoLock();
    
                UUID entity1ID = (UUID)bi.GetUserData(inSubShapePair.GetBody1ID());
                UUID entity2ID = (UUID)bi.GetUserData(inSubShapePair.GetBody2ID());

                if (entity1ID != 0 && entity2ID != 0)
                {
                    std::lock_guard<std::mutex> lock(m_JoltWorld->m_EventQueueMutex);
                    m_JoltWorld->m_CollisionEndEvents.push_back({ entity1ID, entity2ID });
                }
            }
        private:
            Scene* m_Scene = nullptr;
            JoltWorld* m_JoltWorld = nullptr;
        };
        std::vector<CollisionEvent> m_CollisionBeginEvents;
        std::vector<CollisionEvent> m_CollisionEndEvents;
        std::mutex m_EventQueueMutex;
        
        Scene* m_Scene = nullptr;
        JPH::BodyInterface* body_interface;

        Scope<JoltWorldHelper> m_JoltWorldHelper;
        
        JPH::PhysicsSystem physics_system;
        MyContactListener m_ContactListener;
        friend class JoltWorldHelper;
    };
}
