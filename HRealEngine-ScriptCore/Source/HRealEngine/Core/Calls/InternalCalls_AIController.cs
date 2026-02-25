using System.Runtime.CompilerServices;
using HRealEngine.BehaviorTree;

namespace HRealEngine.Calls
{
    public static class InternalCalls_AIController
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static int AIController_GetCurrentPerceptionCount(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static int AIController_GetForgottenPerceptionCount(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static PerceptionResult[] AIController_GetCurrentPerceptions(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static PerceptionResult[] AIController_GetForgottenPerceptions(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool AIController_IsEntityPerceived(ulong entityID, ulong targetEntityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool AIController_IsEntityForgotten(ulong entityID, ulong targetEntityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool AIController_HasBehaviorTree(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static BTBlackboard AIController_GetBlackboard(ulong entityID);
    }
}