
using System;
using HRealEngine.BehaviorTree;

public class Blackboard_Example : BTBlackboard
{
    public Blackboard_Example()
    {
        CreateInt("Health", 100);
        CreateInt("Counter", 0);
        CreateFloat("Speed", 5.0f);
        CreateString("EnemyTag", "Enemy");
        CreateBool("IsAlerted", false);
        CreateBool("HasTarget", false);
    }
}