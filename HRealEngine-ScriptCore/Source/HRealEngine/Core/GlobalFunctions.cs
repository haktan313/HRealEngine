
using System;
using HRealEngine.Calls;

namespace HRealEngine
{
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
    }
}