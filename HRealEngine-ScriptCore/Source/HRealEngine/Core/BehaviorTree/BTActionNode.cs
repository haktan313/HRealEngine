
namespace HRealEngine.BehaviorTree
{
    public abstract class BTActionNode : BTNode
    {
        protected BTActionParams parameters;

        public virtual void SetParameters(BTActionParams param) { parameters = param; }

        public BTActionParams GetParameters() { return parameters; }

        public override void OnStart() { }
        public override void OnFinished() { }
        public override void OnAbort() { }
    }
}