
using System;
using System.Runtime.CompilerServices;

namespace HRealEngine.Calls
{
    public static class InternalCalls_GlobalCalls
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void OpenScene(string scenePath);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float Time_GetDeltaTime();
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void DestroyEntity(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static ulong SpawnEntity(string name, string tag, ref Vector3 translation, ref Vector3 rotation, ref Vector3 scale);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static ulong FindEntityByName(string name);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static object GetScriptInstance(ulong entityID);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool Raycast3D(ref Vector3 origin, ref Vector3 direction, ulong[] ignoreEntitiesIDs, float maxDistance, out ulong entityID, out Vector3 point, out Vector3 normal, out float distance, bool debugDraw, float debugDrawDuration);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static RaycastHit[] Raycast3DArray(ref Vector3 origin, ref Vector3 direction, ulong[] ignoreEntitiesIDs, float maxDistance, bool debugDraw, float debugDrawDuration);
    }
}