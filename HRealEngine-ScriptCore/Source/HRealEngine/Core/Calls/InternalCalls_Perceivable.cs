

using System.Runtime.CompilerServices;

namespace HRealEngine.Calls
{
    public static class InternalCalls_Perceivable
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern internal static void PerceivableComponent_GetType(ulong entityID, out int type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern internal static void PerceivableComponent_SetType(ulong entityID, int type);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern internal static bool PerceivableComponent_GetIsDetectable(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern internal static void PerceivableComponent_SetIsDetectable(ulong entityID, bool isDetectable);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern internal static int PerceivableComponent_GetDetectablePointCount(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern internal static void PerceivableComponent_GetDetectablePoint(ulong entityID, int index, out Vector3 point);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern internal static void PerceivableComponent_SetDetectablePoint(ulong entityID, int index, ref Vector3 point);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern internal static void PerceivableComponent_AddDetectablePoint(ulong entityID, ref Vector3 point);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern internal static void PerceivableComponent_RemoveDetectablePoint(ulong entityID, int index);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern internal static void PerceivableComponent_ClearDetectablePoints(ulong entityID);
    }
}