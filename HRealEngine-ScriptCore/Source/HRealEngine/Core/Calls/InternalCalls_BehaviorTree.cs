
using System.Runtime.CompilerServices;
using HRealEngine.BehaviorTree;

namespace HRealEngine.Calls
{
    public static class InternalCalls_BehaviorTree
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static BTBlackboard BehaviorTreeComponent_GetBlackboard(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Blackboard_NotifyValuesChanged(ulong entityID);
    }
}