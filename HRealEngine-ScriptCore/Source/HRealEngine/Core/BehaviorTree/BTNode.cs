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
        
        public static Entity FromID(ulong entityID)
        {
            if (entityID == 0)
                return null;

            return new Entity(entityID);
        }
        public bool HasTag(string tag)
        {
            return InternalCalls.HasTag(ownerEntityID, tag);
        }
        public Entity FindEntityByName(string name)
        {
            ulong entityID = InternalCalls.FindEntityByName(name);
            if (entityID == 0)
                return null;
            return new Entity(entityID);
        }
        public void Destroy(ulong entityID)
        {
            InternalCalls.DestroyEntity(entityID);
        }
        public void OpenScene(string scenePath)
        {
            InternalCalls.OpenScene(scenePath);
        }
        public Entity SpawnEntity(string name, string tag, Vector3 translation, Vector3 rotation, Vector3 scale)
        {
            ulong entityID = InternalCalls.SpawnEntity(name, tag, ref translation, ref rotation, ref scale);
            if (entityID == 0)
                return null;
            return new Entity(entityID);
        }
        public Entity GetHoveredEntity()
        {
            ulong entityID = InternalCalls.Entity_GetHoveredEntity();
            if (entityID == 0)
                return null;
            return new Entity(entityID);
        }
        public float GetDeltaTime()
        {
            return InternalCalls.Time_GetDeltaTime();
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