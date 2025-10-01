

using System;

namespace HRealEngine
{
    public class Player : Entity
    {
        private TransformComponent m_Transform;
        private Rigidbody2DComponent m_Rigidbody;

        void OnCreate()
        {
            Console.WriteLine($"Player.OnCreate - {EntityID}");

            m_Transform = GetComponent<TransformComponent>();
            m_Rigidbody = GetComponent<Rigidbody2DComponent>();
        }

        void OnUpdate(float ts)
        {
            Console.WriteLine($"Player.OnUpdate: {ts}");

            float speed = 0.01f;
            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCodes.HRE_KEY_UP))
                velocity.Y = 1.0f;
            else if (Input.IsKeyDown(KeyCodes.HRE_KEY_DOWN))
                velocity.Y = -1.0f;

            if (Input.IsKeyDown(KeyCodes.HRE_KEY_LEFT))
                velocity.X = -1.0f;
            else if (Input.IsKeyDown(KeyCodes.HRE_KEY_RIGHT))
                velocity.X = 1.0f;

            velocity *= speed;

            m_Rigidbody.ApplyLinearImpulse(velocity.XY, true);

            //Vector3 translation = m_Transform.Translation;
            //translation += velocity * ts;
            //m_Transform.Translation = translation;
        }
    }
}