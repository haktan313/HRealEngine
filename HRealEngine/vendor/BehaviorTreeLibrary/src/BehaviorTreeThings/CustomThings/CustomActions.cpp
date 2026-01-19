#include <iostream>
#include "CustomActions.h"
#include "BlackboardBase.h"

#include "EnemyAI.h"

void MoveToAction::OnStart()
{
    HActionNode::OnStart();
    m_DistanceToTarget = GetBlackboard().GetFloatValue("DistanceToPlayer");
    std::cout << "MoveToAction started." << std::endl;
    auto owner = GetOwner<EnemyAI>();
    if (owner)
    {
        owner->SetTestValue(3.14f);
        owner->PrintStatus();
        owner->SetTestValue(6.28f);
        float testValue = owner->GetTestValue();
        std::cout << "Test Value after setting: " << testValue << std::endl;
    }
    std::cout << "Initial Distance to Target: " << m_DistanceToTarget << std::endl;
}

NodeStatus MoveToAction::Update()
{
    if (m_DistanceToTarget <= m_StopDistance)
    {
        std::cout << "Reached target. Current Distance: " << m_DistanceToTarget << std::endl;
        GetBlackboard().SetFloatValue("DistanceToPlayer", m_DistanceToTarget);
        return NodeStatus::SUCCESS;
    }
    m_DistanceToTarget -= m_MoveSpeed;
    GetBlackboard().SetFloatValue("DistanceToPlayer", m_DistanceToTarget);
    std::cout << "Moving towards target. Current Distance: " << m_DistanceToTarget << std::endl;
    return NodeStatus::RUNNING;
}

void MoveToAction::OnFinished()
{
    HActionNode::OnFinished();
    std::cout << "MoveToAction finished." << std::endl;
}

void MoveToAction::OnAbort()
{
    HActionNode::OnAbort();
    std::cout << "MoveToAction aborted." << std::endl;
}

//Melee Enemy Actions
//Melee Enemy Attack Action

void MeleeEnemyAttackAction::OnStart()
{
    HActionNode::OnStart();
    GetBlackboard().SetBoolValue("IsAttacking", true);
    std::cout << "MeleeEnemyAttackAction started." << std::endl;
}

NodeStatus MeleeEnemyAttackAction::Update()
{
    m_ElapsedTime += .1f;
    if (m_ElapsedTime >= m_AttackDuration)
    {
        std::cout << "MeleeEnemyAttackAction completed attack." << std::endl;
        int attackPower = GetBlackboard().GetIntValue(m_AttackPowerKey);
        std::cout << "Dealt " << attackPower << " damage to the player." << std::endl;
        GetBlackboard().SetBoolValue("IsAttacking", false);
        m_ElapsedTime = 0.0f;
        return NodeStatus::SUCCESS;
    }
    std::cout << "MeleeEnemyAttackAction attacking... Elapsed Time: " << m_ElapsedTime << std::endl;
    return NodeStatus::RUNNING;
}

void MeleeEnemyAttackAction::OnFinished()
{
    HActionNode::OnFinished();
    m_ElapsedTime = 0.0f;
    std::cout << "MeleeEnemyAttackAction finished." << std::endl;
}

void MeleeEnemyAttackAction::OnAbort()
{
    HActionNode::OnAbort();
    GetBlackboard().SetBoolValue("IsAttacking", false);
    m_ElapsedTime = 0.0f;
    std::cout << "MeleeEnemyAttackAction aborted." << std::endl;
}

void HeavyAttackAction::OnStart()
{
    HActionNode::OnStart();
    GetBlackboard().SetBoolValue("IsAttacking", true);
    float currentStamina = GetBlackboard().GetFloatValue("Stamina");
    std::cout << "HeavyAttackAction started." << "Current Stamina: " << currentStamina << std::endl;
}

NodeStatus HeavyAttackAction::Update()
{
    m_ElapsedTime += .1f;
    if (m_ElapsedTime >= m_AttackDuration)
    {
        int attackPower = GetBlackboard().GetIntValue(m_AttackPowerKey);
        std::cout << "HeavyAttackAction completed heavy attack." << std::endl;
        std::cout << "Dealt " << attackPower << " damage to the player." << std::endl;

        GetBlackboard().SetBoolValue("IsAttacking", false);
        m_ElapsedTime = 0.0f;
        return NodeStatus::SUCCESS;
    }
    std::cout << "HeavyAttackAction performing heavy attack... Elapsed Time: " << m_ElapsedTime << std::endl;
    return NodeStatus::RUNNING;
}

void HeavyAttackAction::OnFinished()
{
    HActionNode::OnFinished();
    m_ElapsedTime = 0.0f;
    std::cout << "HeavyAttackAction finished." << std::endl;
    float currentStamina = GetBlackboard().GetFloatValue(m_StaminaKey);
    currentStamina -= m_StaminaCost;
    GetBlackboard().SetFloatValue(m_StaminaKey, currentStamina);
    std::cout << "Stamina after heavy attack: " << currentStamina << std::endl;
}

void HeavyAttackAction::OnAbort()
{
    HActionNode::OnAbort();
    GetBlackboard().SetBoolValue("IsAttacking", false);
    m_ElapsedTime = 0.0f;
    std::cout << "HeavyAttackAction aborted." << std::endl;
}

