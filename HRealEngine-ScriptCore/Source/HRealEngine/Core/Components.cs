namespace HRealEngine.HRealEngine.Core
{
    public abstract class Component
    {
        public Entity entity { get; internal set; }
    }

    public class TransformComponent : Component
    {
        public Vector3 Translation
        {
            get
            {
                InternalCalls.TransformComponent_GetTranslation(entity.EntityID, out Vector3 result);
                return result;
            }
            set
            {
                InternalCalls.TransformComponent_SetTranslation(entity.EntityID, ref value);
            }
        }
    }
    public class RigidBody2DComponent : Component
    {
        public void ApplyLinearImpulse(Vector2 impulse, Vector2 point, bool wake)
        {
            InternalCalls.RigidBody2DComponent_ApplyLinearImpulse(entity.EntityID, ref impulse, ref point, wake);
        } 
        public void ApplyLinearImpulse(Vector2 impulse, bool wake)
        {
            InternalCalls.RigidBody2DComponent_ApplyLinearImpulse(entity.EntityID, ref impulse, wake);
        }
    }
}