
using System;
using System.Runtime.CompilerServices;

namespace HRealEngine.Calls
{
    public static class InternalCalls_Rigidbody
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulse(ulong entityID, ref Vector2 impulse, ref Vector2 point, bool wake);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(ulong entityID, ref Vector2 impulse, bool wake);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_ApplyLinearImpulseToCenter(ulong entityID, ref Vector3 impulse);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_GetLinearVelocity(ulong entityID, out Vector3 velocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_SetLinearVelocity(ulong entityID, ref Vector3 velocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_SetRotationDegrees(ulong entityID, ref Vector3 eulerDeg);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_GetRotationDegrees(ulong entityID, out Vector3 result);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_SetBodyType(ulong entityID, RigidBodyType bodyType);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_GetBodyType(ulong entityID, out RigidBodyType bodyType);
    }
}