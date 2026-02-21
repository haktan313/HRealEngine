
using System;
using System.Runtime.CompilerServices;

namespace HRealEngine.Calls
{
    public static class InternalCalls_MeshRenderer
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void MeshRendererComponent_SetMesh(ulong entityID, string meshPath);
    }
}