using HRealEngine.Calls;
using System.Runtime.InteropServices;
using HRealEngine.BehaviorTree;

namespace HRealEngine
{
    public abstract class Component
    {
        public Entity entity { get; internal set; }
    }
    
    public class TransformComponent : Component
    {
        public Vector3 Position
        {
            get
            {
                InternalCalls_TransformComponent.TransformComponent_GetPosition(entity.EntityID, out Vector3 result);
                return result;
            }
            set
            {
                InternalCalls_TransformComponent.TransformComponent_SetPosition(entity.EntityID, ref value);
            }
        }
        public Vector3 Rotation
        {
            get
            {
                InternalCalls_TransformComponent.TransformComponent_GetRotation(entity.EntityID, out Vector3 r); 
                return r;
            }
            set
            {
                InternalCalls_TransformComponent.TransformComponent_SetRotation(entity.EntityID, ref value);
            }
        }
    }
    public class Rigidbody2DComponent : Component
    {
        public void ApplyLinearImpulse(Vector2 impulse, Vector2 point, bool wake)
        {
            InternalCalls_Rigidbody.Rigidbody2DComponent_ApplyLinearImpulse(entity.EntityID, ref impulse, ref point, wake);
        } 
        public void ApplyLinearImpulse(Vector2 impulse, bool wake)
        {
            InternalCalls_Rigidbody.Rigidbody2DComponent_ApplyLinearImpulseToCenter(entity.EntityID, ref impulse, wake);
        }
    }
    public class Rigidbody3DComponent : Component
    {
        public void ApplyLinearImpulse(Vector3 impulse)
        {
            InternalCalls_Rigidbody.Rigidbody3DComponent_ApplyLinearImpulseToCenter(entity.EntityID, ref impulse);
        } 
        public Vector3 GetLinearVelocity()
        {
            InternalCalls_Rigidbody.Rigidbody3DComponent_GetLinearVelocity(entity.EntityID, out Vector3 v);
            return v;
        }
        public void SetLinearVelocity(Vector3 v)
        {
            InternalCalls_Rigidbody.Rigidbody3DComponent_SetLinearVelocity(entity.EntityID, ref v);
        }
        public void SetRotationDegrees(Vector3 eulerDeg)
        {
            InternalCalls_Rigidbody.Rigidbody3DComponent_SetRotationDegrees(entity.EntityID, ref eulerDeg);
        }
        public void GetRotationDegrees(out Vector3 rot)
        {
            InternalCalls_Rigidbody.Rigidbody3DComponent_GetRotationDegrees(entity.EntityID, out rot);
        }
        public void SetBodyType(RigidBodyType bodyType)
        {
            InternalCalls_Rigidbody.Rigidbody3DComponent_SetBodyType(entity.EntityID, bodyType);
        }
        public RigidBodyType GetBodyType()
        {
            InternalCalls_Rigidbody.Rigidbody3DComponent_GetBodyType(entity.EntityID, out RigidBodyType bodyType);
            return bodyType;
        }
    }
    
    public class TextComponent : Component
    {

        public string Text
        {
            get => InternalCalls_TextComponent.TextComponent_GetText(entity.EntityID);
            set => InternalCalls_TextComponent.TextComponent_SetText(entity.EntityID, value);
        }

        public Vector4 Color
        {
            get
            {
                InternalCalls_TextComponent.TextComponent_GetColor(entity.EntityID, out Vector4 color);
                return color;
            }

            set
            {
                InternalCalls_TextComponent.TextComponent_SetColor(entity.EntityID, ref value);
            }
        }

        public float Kerning
        {
            get => InternalCalls_TextComponent.TextComponent_GetKerning(entity.EntityID);
            set => InternalCalls_TextComponent.TextComponent_SetKerning(entity.EntityID, value);
        }

        public float LineSpacing
        {
            get => InternalCalls_TextComponent.TextComponent_GetLineSpacing(entity.EntityID);
            set => InternalCalls_TextComponent.TextComponent_SetLineSpacing(entity.EntityID, value);
        }
    }
    
    public class MeshRendererComponent : Component
    {
        public void SetMesh(string meshPath)
        {
            InternalCalls_MeshRenderer.MeshRendererComponent_SetMesh(entity.EntityID, meshPath);
        }
        public void SetPivotOffset(Vector3 offset)
        {
            InternalCalls_MeshRenderer.MeshRendererComponent_SetPivotOffset(entity.EntityID, ref offset);
        }
        public Vector3 GetPivotOffset()
        {           
            return InternalCalls_MeshRenderer.MeshRendererComponent_GetPivotOffset(entity.EntityID);
        }
    }
    
    public class BoxCollider3DComponent : Component
    {
        public void SetSize(Vector3 size)
        {
            InternalCalls_BoxCollider.BoxCollider3DComponent_SetSize(entity.EntityID, ref size);
        }
        public Vector3 GetSize()
        {
            return InternalCalls_BoxCollider.BoxCollider3DComponent_GetSize(entity.EntityID);
        }
        public void SetOffset(Vector3 offset)
        {
            InternalCalls_BoxCollider.BoxCollider3DComponent_SetOffset(entity.EntityID, ref offset);
        }
        public Vector3 GetOffset()
        {
            return InternalCalls_BoxCollider.BoxCollider3DComponent_GetOffset(entity.EntityID);
        }
        public void SetIsTrigger(bool isTrigger)
        {
            InternalCalls_BoxCollider.BoxCollider3DComponent_SetIsTrigger(entity.EntityID, isTrigger);
        }
        public bool GetIsTrigger()
        {
            return InternalCalls_BoxCollider.BoxCollider3DComponent_GetIsTrigger(entity.EntityID);
        }
    }
    
    public enum PerceivableType
    {
        Player = 0,
        Enemy,
        Neutral,
        Environment
    }
    public enum PerceptionMethod
    {
        Sight = 0,
        Hearing,
        Touch
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct PerceptionResult
    {
        public ulong EntityID;
        public int Type;           // PerceivableType
        public int Method;         // PerceptionMethod
        public Vector3 SensedPosition;
        public float TimeSinceLastSensed;
    }
    public class AIControllerComponent : Component
    {
        public int CurrentPerceptionCount()
        {
            return InternalCalls_AIController.AIController_GetCurrentPerceptionCount(entity.EntityID);
        }

        public int ForgottenPerceptionCount()
        {
            return InternalCalls_AIController.AIController_GetForgottenPerceptionCount(entity.EntityID);
        }
    
        public PerceptionResult[] GetCurrentPerceptions()
        {
            return InternalCalls_AIController.AIController_GetCurrentPerceptions(entity.EntityID);
        }
    
        public PerceptionResult[] GetForgottenPerceptions()
        {
            return InternalCalls_AIController.AIController_GetForgottenPerceptions(entity.EntityID);
        }
    
        public bool IsEntityPerceived(ulong targetEntityID)
        {
            return InternalCalls_AIController.AIController_IsEntityPerceived(entity.EntityID, targetEntityID);
        }
    
        public bool IsEntityForgotten(ulong targetEntityID)
        {
            return InternalCalls_AIController.AIController_IsEntityForgotten(entity.EntityID, targetEntityID);
        }
        
        public bool HasBehaviorTree()
        {
            return InternalCalls_AIController.AIController_HasBehaviorTree(entity.EntityID);
        }
        public BTBlackboard GetBlackboard()
        {
            return InternalCalls_AIController.AIController_GetBlackboard(entity.EntityID);
        }
    }
    
    public class BehaviorTreeComponent : Component
    {
        public BTBlackboard GetBlackboard()
        {
            return InternalCalls_BehaviorTree.BehaviorTreeComponent_GetBlackboard(entity.EntityID);
        }
    }
    
    public class PerceivableComponent : Component
    {
        public PerceivableType Type
        {
            get
            {
                InternalCalls_Perceivable.PerceivableComponent_GetType(entity.EntityID, out int type);
                return (PerceivableType)type;
            }
            set
            {
                InternalCalls_Perceivable.PerceivableComponent_SetType(entity.EntityID, (int)value);
            }
        }

        public bool IsDetectable
        {
            get
            {
                return InternalCalls_Perceivable.PerceivableComponent_GetIsDetectable(entity.EntityID);
            }
            set
            {
                InternalCalls_Perceivable.PerceivableComponent_SetIsDetectable(entity.EntityID, value);
            }
        }

        public int DetectablePointCount()
        {
            return InternalCalls_Perceivable.PerceivableComponent_GetDetectablePointCount(entity.EntityID);
        }

        public Vector3 GetDetectablePoint(int index)
        {
            InternalCalls_Perceivable.PerceivableComponent_GetDetectablePoint(entity.EntityID, index, out Vector3 point);
            return point;
        }

        public void SetDetectablePoint(int index, Vector3 point)
        {
            InternalCalls_Perceivable.PerceivableComponent_SetDetectablePoint(entity.EntityID, index, ref point);
        }

        public void AddDetectablePoint(Vector3 point)
        {
            InternalCalls_Perceivable.PerceivableComponent_AddDetectablePoint(entity.EntityID, ref point);
        }

        public void RemoveDetectablePoint(int index)
        {
            InternalCalls_Perceivable.PerceivableComponent_RemoveDetectablePoint(entity.EntityID, index);
        }

        public void ClearDetectablePoints()
        {
            InternalCalls_Perceivable.PerceivableComponent_ClearDetectablePoints(entity.EntityID);
        }

        public Vector3[] GetAllDetectablePoints()
        {
            int count = DetectablePointCount();
            Vector3[] points = new Vector3[count];
            for (int i = 0; i < count; i++)
                points[i] = GetDetectablePoint(i);
            return points;
        }
    }
}