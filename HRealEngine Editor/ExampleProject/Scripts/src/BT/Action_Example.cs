using HRealEngine.BehaviorTree;

public class Action_ExampleParameters : BTActionParams
{
    [BTParameter("Speed")]
    public float DamageAmount = 10.0f;
    [BTParameter("TargetTag")]
    public string TargetTag = "Enemy";
    [BTBlackboardKey(BTBlackboardKeyAttribute.KeyType.Bool, "IsAlertedKey")]
    public string IsAlertedKey = "IsAlerted";
    [BTBlackboardKey(BTBlackboardKeyAttribute.KeyType.Int, "HealthKey")]
    public string HealthKey = "Health";
}
public class Action_Example : BTActionNode
{
    private Action_ExampleParameters parameters;
        
    public Action_Example()
    {
        parameters = new Action_ExampleParameters();
        SetParameters(parameters);
    }
    public override void SetParameters(BTActionParams param)
    {
        base.SetParameters(param);
        parameters = param as Action_ExampleParameters;
    }
    protected override void OnInitialize()
    {
        parameters = parameters as Action_ExampleParameters;
    }
        
    public override void OnStart()
    {
        bool isAlerted = blackboard.GetBool(parameters.IsAlertedKey);
        blackboard.SetInt(parameters.HealthKey, 10);
    }

    public override NodeStatus Update()
    {
        return NodeStatus.Running;
    }

    public override void OnFinished()
    {
            
    }
}