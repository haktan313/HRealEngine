namespace HRealEngine
{
    public class Camera : Entity
    {
        public Entity Target;
        public float DistanceToTarget = 2.0f;
        private Entity m_Player;
        
        void OnCreate()
        {
            m_Player = FindEntityByName("Empty Entity");
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