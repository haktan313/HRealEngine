namespace HRealEngine.BehaviorTree
{
    public abstract class BTDecorator : BTNode
    {
        protected BTDecoratorParams parameters;

        public virtual void SetParameters(BTDecoratorParams param) { parameters = param; }
        public BTDecoratorParams GetParameters() { return parameters; }

        public override void OnStart() { }
        
        public abstract bool CanExecute();
        public abstract void OnFinishedResult(ref NodeStatus status);

        public override NodeStatus Update() { return NodeStatus.Success; }

        public override void OnFinished() { }
        public override void OnAbort() { }
    }
}