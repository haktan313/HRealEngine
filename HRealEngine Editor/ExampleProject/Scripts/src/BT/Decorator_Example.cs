using HRealEngine.BehaviorTree;

public class Decorator_ExampleParameters : BTDecoratorParams
{
    [BTParameter("Threshold")]
    public int threshold = 5;
    [BTBlackboardKey(BTBlackboardKeyAttribute.KeyType.Int, "CounterKey")]
    public string CounterKey = "Counter";
}
public class Decorator_Example : BTDecorator
{
    private Decorator_ExampleParameters parameters;
        
    public Decorator_Example()
    {
        parameters = new Decorator_ExampleParameters();
        SetParameters(parameters);
    }
    public override void SetParameters(BTDecoratorParams param)
    {
        base.SetParameters(param);
        parameters = param as Decorator_ExampleParameters;
    }
    protected override void OnInitialize()
    {
        parameters = parameters as Decorator_ExampleParameters;
    }

    public override void OnStart()
    {
            
    }
    public override bool CanExecute()
    {
        return blackboard.GetInt(parameters.CounterKey) < parameters.threshold;
    }
    public override void OnFinishedResult(ref NodeStatus status)
    {
        if (status == NodeStatus.Success)
        {
            int currentCounter = blackboard.GetInt(parameters.CounterKey);
            blackboard.SetInt(parameters.CounterKey, currentCounter + 1);
        }
    }
    public override void OnFinished()
    {

    }
}