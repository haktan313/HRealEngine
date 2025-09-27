

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

        Entity& GetEntity() { return m_Entity; }
        void DestroySelf() { m_Entity.Destoy(); }
    private:
        Entity m_Entity;
        friend class Scene;
    };
}
