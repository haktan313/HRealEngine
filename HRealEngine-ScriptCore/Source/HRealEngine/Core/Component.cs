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
        public Vector3 Rotation
        {
            get
            {
                InternalCalls.TransformComponent_GetRotation(entity.EntityID, out Vector3 r); 
                return r;
            }
            set
            {
                InternalCalls.TransformComponent_SetRotation(entity.EntityID, ref value);
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
    
    public class TextComponent : Component
    {

        public string Text
        {
            get => InternalCalls.TextComponent_GetText(entity.EntityID);
            set => InternalCalls.TextComponent_SetText(entity.EntityID, value);
        }

        public Vector4 Color
        {
            get
            {
                InternalCalls.TextComponent_GetColor(entity.EntityID, out Vector4 color);
                return color;
            }

            set
            {
                InternalCalls.TextComponent_SetColor(entity.EntityID, ref value);
            }
        }

        public float Kerning
        {
            get => InternalCalls.TextComponent_GetKerning(entity.EntityID);
            set => InternalCalls.TextComponent_SetKerning(entity.EntityID, value);
        }

        public float LineSpacing
        {
            get => InternalCalls.TextComponent_GetLineSpacing(entity.EntityID);
            set => InternalCalls.TextComponent_SetLineSpacing(entity.EntityID, value);
        }

    }
}