

using System;
using System.Runtime.CompilerServices;

namespace HRealEngine
{
    public static class InternalCalls
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(ulong entityID, Type componentType);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(KeyCodes keyCode);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetTranslation(ulong entityID, out Vector3 result);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetTranslation(ulong entityID, ref Vector3 value);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void RigidBody2DComponent_ApplyLinearImpulse(ulong entityID, ref Vector2 impulse, ref Vector2 point, bool wake);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void RigidBody2DComponent_ApplyLinearImpulse(ulong entityID, ref Vector2 impulse, bool wake);
    }
}