
using System;
using System.Runtime.CompilerServices;

namespace HRealEngine.Calls
{
    public static class InternalCalls_Entity
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Entity_AddComponent(ulong entityID, Type componentType);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Entity_AddRigidbody3DComponent(ulong entityID, RigidBodyType bodyType, bool fixedRotation, float friction, float restitution, float convexRadius);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Entity_AddBoxCollider3DComponent(ulong entityID, bool isTrigger, ref Vector3 size, ref Vector3 offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Entity_AddMeshRendererComponent(ulong entityID, string meshPath);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static string Entity_GetName(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]  
        internal extern static void Entity_SetName(ulong entityID, string name);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(ulong entityID, Type componentType);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasTag(ulong entityID, string tag);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static ulong Entity_GetHoveredEntity();
    }
}