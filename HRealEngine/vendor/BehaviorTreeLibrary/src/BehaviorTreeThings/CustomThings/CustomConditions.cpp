#include "CustomConditions.h"

#include <iostream>

#include "BlackboardBase.h"


void IsPlayerInRangeCondition::OnStart()
{
    HCondition::OnStart();
    std::cout << "IsPlayerInRangeCondition started.\n";
    float distance = GetBlackboard().GetFloatValue(m_DistanceToPlayer);
    std::cout << "Distance to player from blackboard: " << distance << "\n";
}

bool IsPlayerInRangeCondition::CheckCondition()
{
    if (GetBlackboard().GetFloatValue("DistanceToPlayer") <= m_Range)
    {
        std::cout << "Player is within range (" << m_Range << ").\n";
        GetBlackboard().SetBoolValue("IsPlayerInRange", true);
        return true;
    }
    std::cout << "Player is out of range (" << m_Range << ").\n";
    GetBlackboard().SetBoolValue("IsPlayerInRange", false);
    return false;
}

void IsPlayerInRangeCondition::OnFinished()
{
    HCondition::OnFinished();
    std::cout << "IsPlayerInRangeCondition finished.\n";
}

void IsPlayerInRangeCondition::OnAbort()
{
    HCondition::OnAbort();
    std::cout << "IsPlayerInRangeCondition aborted.\n";
}

void CanAttackCondition::OnStart()
{
    HCondition::OnStart();
}

bool CanAttackCondition::CheckCondition()
{
    float currentStamina = GetBlackboard().GetFloatValue(m_StaminaKey);
    if (currentStamina >= m_RequiredStamina)
        return true;
    
    return false;
}

void CanAttackCondition::OnFinished()
{
    HCondition::OnFinished();
}

void CanAttackCondition::OnAbort()
{
    HCondition::OnAbort();
}
