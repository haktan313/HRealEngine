using HRealEngine.BehaviorTree;

public class Condition_ExampleParameters : BTConditionParams
{
    [BTBlackboardKey(BTBlackboardKeyAttribute.KeyType.Bool, "HasTargetKey")]
    public string HasTargetKey = "HasTarget";
}
public class Condition_Example : BTCondition
{
    private Condition_ExampleParameters parameters;
        
    public Condition_Example()
    {
        parameters = new Condition_ExampleParameters();
        SetParameters(parameters);
    }
    public override void SetParameters(BTConditionParams param)
    {
        base.SetParameters(param);
        parameters = param as Condition_ExampleParameters;
    }
    protected override void OnInitialize()
    {
        parameters = parameters as Condition_ExampleParameters;
    }
        
    public override void OnStart()
    {
            
    }
    public override bool CheckCondition()
    {
        return blackboard.GetBool(parameters.HasTargetKey);
    }
    public override void OnFinished()
    {
            
    }
}