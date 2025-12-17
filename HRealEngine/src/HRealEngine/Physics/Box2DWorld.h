#pragma once

#include "box2d/b2_world.h"
#include "HRealEngine/Core/Entity.h"

namespace HRealEngine
{
    class Scene;
    class Entity;
    class Timestep;
    
    class Box2DWorld
    {
    public:
        Box2DWorld(Scene* scene);
        ~Box2DWorld();
        void Init();
        
        void UpdateSimulation2D(Timestep deltaTime, int& stepFrames);
        void UpdateRuntime2D();
        void Step2DWorld(Timestep deltaTime);
        void DestroyEntityPhysics(Entity entity);
        void Stop2DPhysics();

    private:
        b2World* m_PhysicsWorld2D = nullptr;
        Scene* m_Scene = nullptr;

        struct CollisionEvent
        {
            entt::entity A;
            entt::entity B;
        };
        class GameContactListener : public b2ContactListener
        {
        public:
            explicit GameContactListener(Scene* scene, Box2DWorld* world) : m_Scene(scene), m_Box2DWorld(world) {}
            void BeginContact(b2Contact* contact) override;
            void EndContact(b2Contact* contact) override;
        private:
            Scene* m_Scene;
            Box2DWorld* m_Box2DWorld;
        };
        //GameContactListener m_ContactListener{m_Scene, this};
        std::vector<CollisionEvent> m_CollisionBeginEvents;
        std::vector<CollisionEvent> m_CollisionEndEvents;
    };
}
