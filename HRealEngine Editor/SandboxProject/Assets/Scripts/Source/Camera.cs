
using System;

namespace HRealEngine
{
    public class Camera : Entity
    {
        public Entity Target;
        private Entity m_Player;
        
        public float DistanceToTarget = 2.0f;
        public short ZoomLevel = 1;
        public long ZoomSpeed = 1;
        public int speed = 5;
        public string playerName = "player";
        public bool isActive = true;
        public Vector3 offset = new Vector3(0.0f, 0.0f, 0.0f);

        void OnCreate()
        { 
           m_Player = FindEntityByName(playerName);
           Console.WriteLine($"Camera.OnCreate - {EntityID}");
           Console.WriteLine($"Camera.OnCreate - playerName: {playerName}");
           Console.WriteLine($"Camera.OnCreate - player: {m_Player.EntityID}");
        }
        void OnUpdate(float ts)
        {
            if(m_Player != null)
                Translation = new Vector3(m_Player.Translation.XY, DistanceToTarget);
            
            float speed = 1.0f;
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

            Vector3 translation = Translation;
            translation += velocity * ts;
            Translation = translation;
        }
    }
}