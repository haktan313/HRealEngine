#pragma once
#include "BlackboardBase.h"
#include "Nodes.h"

struct IsPlayerInRangeParameters : ParamsForCondition
{
    float Range = 100.0f;
    HBlackboardKeyValue DistanceToPlayerKey;
    void DrawImGui(HBlackboard* blackboard) override
    {
        DrawFloatValue("Range", Range);
        DrawBlackboardFloatKeySelector("Distance To Player", DistanceToPlayerKey, blackboard);
    }
    void Serialize(YAML::Emitter& out) const override
    {
        SerializeFloat("Range", Range, out);
        SerializeBlackboardFloatKey("DistanceToPlayerKey", DistanceToPlayerKey, out);
    }
    void Deserialize(const YAML::Node& node) override
    {
        DeserializeFloat(node, "Range", Range);
        DeserializeBlackboardKey(node, "DistanceToPlayerKey", DistanceToPlayerKey);
    }
};
class IsPlayerInRangeCondition : public HCondition
{
public:
    IsPlayerInRangeCondition(const std::string& name, const IsPlayerInRangeParameters& params = IsPlayerInRangeParameters{})
        : HCondition(name, params), m_Range(params.Range), m_DistanceToPlayer(params.DistanceToPlayerKey)
    {
        SetParams<IsPlayerInRangeParameters>(params);
    }

    void OnStart() override;
    bool CheckCondition() override;
    void OnFinished() override;
    void OnAbort() override;
private:
    float m_Range;
    HBlackboardKeyValue m_DistanceToPlayer;
};

struct CanAttackParameters : ParamsForCondition
{
    HBlackboardKeyValue StaminaKey;
    float RequiredStamina = 20.0f;
    void DrawImGui(HBlackboard* blackboard) override
    {
        DrawBlackboardFloatKeySelector("Stamina", StaminaKey, blackboard);
        DrawFloatValue("Required Stamina", RequiredStamina);
    }
    void Serialize(YAML::Emitter& out) const override
    {
        SerializeBlackboardFloatKey("StaminaKey", StaminaKey, out);
        SerializeFloat("RequiredStamina", RequiredStamina, out);
    }
    void Deserialize(const YAML::Node& node) override
    {
        DeserializeBlackboardKey(node, "StaminaKey", StaminaKey);
        DeserializeFloat(node, "RequiredStamina", RequiredStamina);
    }
};
class CanAttackCondition : public HCondition
{
public:
    CanAttackCondition(const std::string& name, const CanAttackParameters& params = CanAttackParameters{})
        : HCondition(name, params), m_StaminaKey(params.StaminaKey), m_RequiredStamina(params.RequiredStamina)
    {
        SetParams<CanAttackParameters>(params);
    }

    void OnStart() override;
    bool CheckCondition() override;
    void OnFinished() override;
    void OnAbort() override;
private:
    HBlackboardKeyValue m_StaminaKey;
    float m_RequiredStamina;
};