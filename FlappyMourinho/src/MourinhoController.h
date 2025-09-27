#pragma once
#include "Box2D/include/box2d/b2_body.h"
#include "HRealEngine/Scene/ScriptableEntity.h"

namespace HRealEngine
{
    class MourinhoController : public ScriptableEntity
    {
    public:
        ~MourinhoController() override;
    protected:
        void OnCreate() override;
        void OnDestroy() override;
        void OnUpdate(Timestep ts) override;
        void OnCollisionEnter(Entity other) override;

    private:
        b2Body* m_Body = nullptr;
        float m_JumpForce = 4.f;
        bool m_Jumped = false;
    };
}
