#pragma once
#include "Box2D/include/box2d/b2_body.h"
#include "HRealEngine/Scene/ScriptableEntity.h"

namespace HRealEngine
{
    class aliPipe : public ScriptableEntity
    {
    public:
        ~aliPipe() override;
    protected:
        void OnCreate() override;
        void OnDestroy() override;
        void OnUpdate(Timestep ts) override;
    private:
        b2Body* m_Body = nullptr;
        float m_SpeedOfAli = 1.0f;
    };
}
