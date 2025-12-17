namespace HRealEngine
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
    public class Rigidbody2DComponent : Component
    {
        public void ApplyLinearImpulse(Vector2 impulse, Vector2 point, bool wake)
        {
            InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(entity.EntityID, ref impulse, ref point, wake);
        } 
        public void ApplyLinearImpulse(Vector2 impulse, bool wake)
        {
            InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseToCenter(entity.EntityID, ref impulse, wake);
        }
    }
    public class Rigidbody3DComponent : Component
    {
        public void ApplyLinearImpulse(Vector3 impulse)
        {
            InternalCalls.Rigidbody3DComponent_ApplyLinearImpulseToCenter(entity.EntityID, ref impulse);
        } 
    }
}