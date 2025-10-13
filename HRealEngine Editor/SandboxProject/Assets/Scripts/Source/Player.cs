

using System;

namespace HRealEngine
{
    public class Player : Entity
    {
        private TransformComponent m_Transform;
        private Rigidbody2DComponent m_Rigidbody;
        
        public float Speed = 15.0f;
        public float JumpForce = 5.0f;
        public string cameraName = "camera";

        void OnCreate()
        {
            Console.WriteLine($"Player.OnCreate - {EntityID}");

            m_Transform = GetComponent<TransformComponent>();
            m_Rigidbody = GetComponent<Rigidbody2DComponent>();
        }
        void OnDestroy()
        {
            Console.WriteLine($"Player.OnDestroy - {EntityID}");
        }
        void OnUpdate(float ts)
        {
            //Console.WriteLine($"Player.OnUpdate: {ts}");

            float speed = 0.01f;
            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCodes.HRE_KEY_UP))
                velocity.Y = 10.0f;
            else if (Input.IsKeyDown(KeyCodes.HRE_KEY_DOWN))
                velocity.Y = -10.0f;

            if (Input.IsKeyDown(KeyCodes.HRE_KEY_LEFT))
                velocity.X = -10.0f;
            else if (Input.IsKeyDown(KeyCodes.HRE_KEY_RIGHT))
                velocity.X = 10.0f;
            
            Entity camera = FindEntityByName(cameraName);
            if (camera != null)
            {
                Camera cam = camera.As<Camera>();
                if(Input.IsKeyDown(KeyCodes.HRE_KEY_Q))
                    cam.DistanceToTarget += 2.0f * speed * ts;
                else if(Input.IsKeyDown(KeyCodes.HRE_KEY_E))
                    cam.DistanceToTarget -= 2.0f * speed * ts;
            }

            velocity *= speed;

            m_Rigidbody.ApplyLinearImpulse(velocity.XY, true);

            //Vector3 translation = m_Transform.Translation;
            //translation += velocity * ts;
            //m_Transform.Translation = translation;
        }
        
        void OnCollisionEnter2D(ulong otherID)
        {
            Console.WriteLine($"Player.OnCollisionEnter2D - {otherID}");
            //Destroy(otherID);
        }
        void OnCollisionExit2D(ulong otherID)
        {

        }
    }
}