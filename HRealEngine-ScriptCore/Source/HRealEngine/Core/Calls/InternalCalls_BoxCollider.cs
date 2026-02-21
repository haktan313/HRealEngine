
using System;
using System.Runtime.CompilerServices;

namespace HRealEngine.Calls
{
    public static class InternalCalls_BoxCollider
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider3DComponent_SetSize(ulong entityID, ref Vector3 size);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static Vector3 BoxCollider3DComponent_GetSize(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider3DComponent_SetOffset(ulong entityID, ref Vector3 offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static Vector3 BoxCollider3DComponent_GetOffset(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider3DComponent_SetIsTrigger(ulong entityID, bool isTrigger);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool BoxCollider3DComponent_GetIsTrigger(ulong entityID);
    }
}