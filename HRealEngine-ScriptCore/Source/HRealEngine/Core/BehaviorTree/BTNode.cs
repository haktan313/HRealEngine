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
        
        
        public Entity FromID(ulong entityID)
        {
            return GlobalFunctions.FromID(entityID);
        }
        public float GetDeltaTime()
        {
            return GlobalFunctions.GetDeltaTime();
        }
        public void OpenScene(string scenePath)
        {
            GlobalFunctions.OpenScene(scenePath);
        }
        public void Destroy(ulong entityID)
        {
            GlobalFunctions.Destroy(entityID);
        }
        public Entity SpawnEntity(string name, string tag, Vector3 translation, Vector3 rotation, Vector3 scale)
        {
            return GlobalFunctions.SpawnEntity(name, tag, translation, rotation, scale);
        }
        public Entity FindEntityByName(string name)
        {
            return GlobalFunctions.FindEntityByName(name);
        }
    }
}