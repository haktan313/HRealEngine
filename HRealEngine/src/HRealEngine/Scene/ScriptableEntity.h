

#pragma once
#include "HRealEngine/Core/Entity.h"

namespace HRealEngine
{
    class ScriptableEntity
    {
    public:
        virtual ~ScriptableEntity() = default;
        template<typename T>
        T& GetComponent()
        {
            return m_Entity.GetComponent<T>();
        }
    protected:
        virtual void OnCreate() {}
        virtual void OnDestroy() {}
        virtual void OnUpdate(Timestep ts) {}
        
        virtual void OnCollisionBegin(Entity other) {}
        virtual void OnCollisionEnd(Entity other) {}

        Entity& GetEntity() { return m_Entity; }
        void DestroySelf() { m_Entity.Destroy(); }
    private:
        Entity m_Entity;
        friend class Scene;
        friend class Box2DWorld;
        friend class JoltWorld;
    };
}
