using System;

namespace HRealEngine
{
    public enum BTNodeStatus
    {
        Success = 0,
        Failure = 1,
        Running = 2
    }

    public abstract class BTNodeBase
    {
        protected Entity Owner
        {
            get
            {
                ulong ownerId = InternalCalls.BehaviorTree_GetCurrentOwnerEntity();
                return ownerId == 0 ? null : new Entity(ownerId);
            }
        }
    }

    public abstract class BTAction : BTNodeBase
    {
        public virtual void OnStart() { }
        public abstract BTNodeStatus OnUpdate();
        public virtual void OnFinished() { }
        public virtual void OnAbort() { }
    }

    public abstract class BTCondition : BTNodeBase
    {
        public abstract bool CheckCondition();
    }

    public abstract class BTDecorator : BTNodeBase
    {
        public virtual bool CanExecute() => true;
        public virtual BTNodeStatus OnFinishedResult(BTNodeStatus status) => status;
    }

    public abstract class BTBlackboard : BTNodeBase
    {
        public abstract void OnCreate();

        protected void CreateBool(string key, bool value) => InternalCalls.BehaviorTree_BlackboardCreateBool(key, value);
        protected void CreateInt(string key, int value) => InternalCalls.BehaviorTree_BlackboardCreateInt(key, value);
        protected void CreateFloat(string key, float value) => InternalCalls.BehaviorTree_BlackboardCreateFloat(key, value);
        protected void CreateString(string key, string value) => InternalCalls.BehaviorTree_BlackboardCreateString(key, value);
    }

    public static class BehaviorTreeRegistry
    {
        public static void RegisterAction<T>(string displayName = null) where T : BTAction
        {
            string className = typeof(T).FullName;
            string name = string.IsNullOrWhiteSpace(displayName) ? typeof(T).Name : displayName;
            InternalCalls.BehaviorTree_RegisterAction(name, className);
        }

        public static void RegisterCondition<T>(string displayName = null) where T : BTCondition
        {
            string className = typeof(T).FullName;
            string name = string.IsNullOrWhiteSpace(displayName) ? typeof(T).Name : displayName;
            InternalCalls.BehaviorTree_RegisterCondition(name, className);
        }

        public static void RegisterDecorator<T>(string displayName = null) where T : BTDecorator
        {
            string className = typeof(T).FullName;
            string name = string.IsNullOrWhiteSpace(displayName) ? typeof(T).Name : displayName;
            InternalCalls.BehaviorTree_RegisterDecorator(name, className);
        }

        public static void RegisterBlackboard<T>(string displayName = null) where T : BTBlackboard
        {
            string className = typeof(T).FullName;
            string name = string.IsNullOrWhiteSpace(displayName) ? typeof(T).Name : displayName;
            InternalCalls.BehaviorTree_RegisterBlackboard(name, className);
        }
    }
}
