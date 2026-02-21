
using System;
using System.Runtime.CompilerServices;

namespace HRealEngine.Calls
{
    public static class InternalCalls_MeshRenderer
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void MeshRendererComponent_SetMesh(ulong entityID, string meshPath);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void MeshRendererComponent_SetPivotOffset(ulong entityID, ref Vector3 offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static Vector3 MeshRendererComponent_GetPivotOffset(ulong entityID);
    }
}