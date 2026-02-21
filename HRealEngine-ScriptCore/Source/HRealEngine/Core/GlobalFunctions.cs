
using System;
using HRealEngine.Calls;

namespace HRealEngine
{
    [System.Runtime.InteropServices.StructLayout(System.Runtime.InteropServices.LayoutKind.Sequential)]
    public struct RaycastHit
    {
        public ulong EntityID;
        public Vector3 Point;
        public Vector3 Normal;
        public float Distance;
    }
    
    public static class GlobalFunctions
    {
        public static Entity FromID(ulong entityID)
        {
            if (entityID == 0)
                return null;

            return new Entity(entityID);
        }
        public static float GetDeltaTime()
        {
            return InternalCalls_GlobalCalls.Time_GetDeltaTime();
        }
        public static void OpenScene(string scenePath)
        {
            InternalCalls_GlobalCalls.OpenScene(scenePath);
        }
        public static void Destroy(ulong entityID)
        {
            InternalCalls_GlobalCalls.DestroyEntity(entityID);
        }
        public static Entity SpawnEntity(string name, string tag, Vector3 translation, Vector3 rotation, Vector3 scale)
        {
            ulong entityID = InternalCalls_GlobalCalls.SpawnEntity(name, tag, ref translation, ref rotation, ref scale);
            if (entityID == 0)
                return null;
            return new Entity(entityID);
        }
        public static Entity FindEntityByName(string name)
        {
            ulong entityID = InternalCalls_GlobalCalls.FindEntityByName(name);
            if (entityID == 0)
                return null;
            return new Entity(entityID);
        }
        public static bool Raycast3D(Vector3 origin, Vector3 direction, float maxDistance, out RaycastHit hit, ulong[] ignoreEntities = null, bool debugDraw = false, float debugDrawDuration = 0.0f)
        {
            hit = new RaycastHit();
            bool result = InternalCalls_GlobalCalls.Raycast3D(ref origin, ref direction, ignoreEntities, maxDistance, out hit.EntityID, out hit.Point, out hit.Normal, out hit.Distance, debugDraw, debugDrawDuration);
            return result;
        }

        public static RaycastHit[] Raycast3DAll(Vector3 origin, Vector3 direction, float maxDistance, ulong[] ignoreEntities = null, bool debugDraw = false, float debugDrawDuration = 0.0f)
        {
            return InternalCalls_GlobalCalls.Raycast3DArray(ref origin, ref direction, ignoreEntities, maxDistance, debugDraw, debugDrawDuration);
        }
    }
}