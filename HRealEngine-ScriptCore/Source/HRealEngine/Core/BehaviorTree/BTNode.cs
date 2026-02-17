namespace HRealEngine.BehaviorTree
{
    public enum NodeStatus
    {
        Success = 0,
        Failure = 1,
        Running = 2
    }

    public enum PriorityType
    {
        None = 0,
        Self = 1,
        LowerPriority = 2,
        Both = 3
    }

    public abstract class BTNode
    {
        protected BTBlackboard blackboard;
        protected ulong ownerEntityID;
        
        private Entity cachedOwner;
        
        protected Entity owner
        {
            get
            {
                if (cachedOwner == null || cachedOwner.EntityID != ownerEntityID)
                {
                    cachedOwner = new Entity(ownerEntityID);
                }
                return cachedOwner;
            }
        }
        
        public virtual void Initialize(BTBlackboard bb, ulong entityID)
        {
            blackboard = bb;
            ownerEntityID = entityID;
            OnInitialize();
        }

        protected virtual void OnInitialize() { }

        public abstract void OnStart();
        public abstract NodeStatus Update();
        public abstract void OnFinished();
        public abstract void OnAbort();
    }
}