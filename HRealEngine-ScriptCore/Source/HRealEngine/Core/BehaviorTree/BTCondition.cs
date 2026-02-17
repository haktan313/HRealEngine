namespace HRealEngine.BehaviorTree
{
    public abstract class BTCondition : BTNode
    {
        public PriorityType Priority { get; set; } = PriorityType.None;
        
        protected BTConditionParams parameters;

        public virtual void SetParameters(BTConditionParams param) { parameters = param; }
        public BTConditionParams GetParameters() { return parameters; }

        public override void OnStart() { }
        
        public abstract bool CheckCondition();

        public override NodeStatus Update() { return CheckCondition() ? NodeStatus.Success : NodeStatus.Failure; }

        public override void OnFinished() { }
        public override void OnAbort() { }
    }
}