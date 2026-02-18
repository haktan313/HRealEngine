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
        public Vector3 GetLinearVelocity()
        {
            InternalCalls.Rigidbody3DComponent_GetLinearVelocity(entity.EntityID, out Vector3 v);
            return v;
        }
        public void SetLinearVelocity(Vector3 v)
        {
            InternalCalls.Rigidbody3DComponent_SetLinearVelocity(entity.EntityID, ref v);
        }
        public void SetRotationDegrees(Vector3 eulerDeg)
        {
            InternalCalls.Rigidbody3DComponent_SetRotationDegrees(entity.EntityID, ref eulerDeg);
        }
        public void GetRotationDegrees(out Vector3 rot)
        {
            InternalCalls.Rigidbody3DComponent_GetRotationDegrees(entity.EntityID, out rot);
        }
        public void SetBodyType(RigidBodyType bodyType)
        {
            InternalCalls.Rigidbody3DComponent_SetBodyType(entity.EntityID, bodyType);
        }
        public RigidBodyType GetBodyType()
        {
            InternalCalls.Rigidbody3DComponent_GetBodyType(entity.EntityID, out RigidBodyType bodyType);
            return bodyType;
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
    
    public class MeshRendererComponent : Component
    {
        public void SetMesh(string meshPath)
        {
            InternalCalls.MeshRendererComponent_SetMesh(entity.EntityID, meshPath);
        }
    }
    
    public class BoxCollider3DComponent : Component
    {
        public void SetSize(Vector3 size)
        {
            InternalCalls.BoxCollider3DComponent_SetSize(entity.EntityID, ref size);
        }
        public Vector3 GetSize()
        {
            return InternalCalls.BoxCollider3DComponent_GetSize(entity.EntityID);
        }
        public void SetOffset(Vector3 offset)
        {
            InternalCalls.BoxCollider3DComponent_SetOffset(entity.EntityID, ref offset);
        }
        public Vector3 GetOffset()
        {
            return InternalCalls.BoxCollider3DComponent_GetOffset(entity.EntityID);
        }
        public void SetIsTrigger(bool isTrigger)
        {
            InternalCalls.BoxCollider3DComponent_SetIsTrigger(entity.EntityID, isTrigger);
        }
        public bool GetIsTrigger()
        {
            return InternalCalls.BoxCollider3DComponent_GetIsTrigger(entity.EntityID);
        }
    }
}