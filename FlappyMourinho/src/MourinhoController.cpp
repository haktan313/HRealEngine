#include "MourinhoController.h"

#include "HRealEngine/Core/Input.h"
#include "HRealEngine/Core/KeyCodes.h"
#include "HRealEngine/Core/Logger.h"

namespace HRealEngine
{
    MourinhoController::~MourinhoController()
    {
    }

    void MourinhoController::OnCreate()
    {
        LOG_CORE_INFO("PlayerController::OnCreate");
        auto& rb2d = GetComponent<Rigidbody2DComponent>();
        m_Body = (b2Body*)rb2d.RuntimeBody;
    }

    void MourinhoController::OnDestroy()
    {
        
    }

    void MourinhoController::OnUpdate(Timestep ts)
    {
        if (Input::IsKeyPressed(HR_KEY_SPACE) && !m_Jumped)
        {
            b2Vec2 velocity = m_Body->GetLinearVelocity();
            velocity.y = m_JumpForce;
            m_Body->SetLinearVelocity(velocity);
        }
        m_Jumped = Input::IsKeyPressed(HR_KEY_SPACE);
    }

    void MourinhoController::OnCollisionEnter(Entity other)
    {
        LOG_CORE_INFO("Collision with {0}", other.GetName());
        DestroySelf();
    }
}
