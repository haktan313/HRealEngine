#pragma once
#include "JoltWorldHelper.h"

namespace HRealEngine
{
    class Scene;
    class Entity;
    class Timestep;
    
    class JoltWorld
    {
    public:
        JoltWorld(Scene* scene);
        ~JoltWorld();
        void Init();

        void DestroyEntityPhysics(Entity entity);
        void Step3DWorld(Timestep deltaTime);
        void UpdateSimulation3D(Timestep deltaTime, int& stepFrames);
        
    private:
        Scene* m_Scene = nullptr;
        JPH::BodyInterface* body_interface;

        Scope<JoltWorldHelper> m_JoltWorldHelper;
        
        JPH::PhysicsSystem physics_system;
    };
}
