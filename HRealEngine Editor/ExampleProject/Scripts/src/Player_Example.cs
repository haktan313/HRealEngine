using System;
using HRealEngine;

namespace ExampleProject
{
    public class Player_Example : Entity
    {
        private float moveSpeed = 5.0f;
        private Rigidbody3DComponent rigidbody;
        
        private float noiseTimer = 0.0f;
        private float elapsedTime = 0.0f;
        void BeginPlay()
        {
            rigidbody = GetComponent<Rigidbody3DComponent>();
        }
        void OnDestroy()
        {
            
        }
        void Tick(float ts)
        {
            Vector3 input = new Vector3();
            if (Input.IsKeyDown(KeyCodes.HRE_KEY_W))
                input.Z -= 1;
            if (Input.IsKeyDown(KeyCodes.HRE_KEY_S))
                input.Z += 1;
            if (Input.IsKeyDown(KeyCodes.HRE_KEY_A))
                input.X -= 1;
            if (Input.IsKeyDown(KeyCodes.HRE_KEY_D))
                input.X += 1;

            if (input.Length() > 0)
            {
                input = input.Normalized() * moveSpeed;
                rigidbody.SetLinearVelocity(input);
            }
            else
                rigidbody.SetLinearVelocity(new Vector3(0, rigidbody.GetLinearVelocity().Y, 0));
            
            if (Input.IsKeyDown(KeyCodes.HRE_KEY_SPACE))
            {
                Console.WriteLine("Player is making noise!");
                ReportNoiseEvent(EntityID, Position, 1.0f, 10.0f, 0);
            }
        }
        void OnCollisionEnter(ulong otherEntityID)
        {
            
        }
        void OnCollisionExit(ulong otherEntityID)
        {
            
        }
    }
}