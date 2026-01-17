#pragma once
#include "BlackboardBase.h"

class MeleeEnemyBlackboard : public HBlackboard
{
public:
    MeleeEnemyBlackboard(const std::string& name = "MeleeEnemyBlackboard") : HBlackboard(name)
    {
        CreateBoolValue("IsPlayerInRange", false);
        CreateBoolValue("IsPlayerAttacking", false);
        CreateBoolValue("CanAttack", true);
        CreateBoolValue("IsAttacking", false);
        
        CreateFloatValue("DistanceToPlayer", 200.0f);
        CreateFloatValue("Health", 100.0f);
        CreateFloatValue("Stamina", 50.0f);
        
        CreateIntValue("AttackPower", 10);
        
        CreateStringValue("CurrentState", "Idle");
    }
};

class RangedEnemyBlackboard : public HBlackboard
{
public:
    RangedEnemyBlackboard(const std::string& name = "RangedEnemyBlackboard") : HBlackboard(name)
    {
        CreateBoolValue("IsPlayerInSight", false);
        CreateBoolValue("ShouldReload", false);
        CreateBoolValue("CanShoot", true);

        CreateFloatValue("DistanceToPlayer", 300.0f);
        CreateFloatValue("Health", 100.0f);
        
        CreateIntValue("AmmoCount", 5);
        CreateIntValue("DamageAmount", 15);

        CreateStringValue("CurrentState", "Idle");
        
    }
};