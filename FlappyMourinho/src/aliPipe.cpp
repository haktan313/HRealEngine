#include "aliPipe.h"

namespace HRealEngine
{
    aliPipe::~aliPipe()
    {
    }

    void aliPipe::OnCreate()
    {
        auto& rb2d = GetComponent<Rigidbody2DComponent>();
        m_Body = (b2Body*)rb2d.RuntimeBody;
    }

    void aliPipe::OnDestroy()
    {
        
    }

    void aliPipe::OnUpdate(Timestep ts)
    {
        m_Body->SetLinearVelocity(b2Vec2(-m_SpeedOfAli, 0.0f));

        const float despawnX = -5.0f;
        if (m_Body->GetPosition().x < despawnX)
            DestroySelf();
    }
}
