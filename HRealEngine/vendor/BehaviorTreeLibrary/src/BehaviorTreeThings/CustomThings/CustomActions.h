#pragma once
#include "BTSerializer.h"
#include "imgui.h"
#include "Nodes.h"
#include <yaml-cpp/yaml.h>

#include "NodeRegistry.h"

struct MoveToParameters : ParamsForAction
{
    bool bToPlayer = true;
    float StopDistance = 10.0f;
    float MoveSpeed = 5.0f;
    
    void DrawImGui(HBlackboard* blackboard) override
    {
        DrawBoolValue("Move To Player", bToPlayer);
        DrawFloatValue("Stop Distance", StopDistance);
        DrawFloatValue("Move Speed", MoveSpeed);
    }
    void Serialize(YAML::Emitter& out) const override
    {
        SerializeBool("bToPlayer", bToPlayer, out);
        SerializeFloat("StopDistance", StopDistance, out);
        SerializeFloat("MoveSpeed", MoveSpeed, out);
    }
    void Deserialize(const YAML::Node& node)
    {
        DeserializeBool(node, "bToPlayer", bToPlayer);
        DeserializeFloat(node, "StopDistance", StopDistance);
        DeserializeFloat(node, "MoveSpeed", MoveSpeed);
    }
};
class MoveToAction : public HActionNode
{
public:
    MoveToAction(const std::string& name, const MoveToParameters& params = MoveToParameters{})
        : HActionNode(name, params), m_bToPlayer(params.bToPlayer), m_StopDistance(params.StopDistance), m_MoveSpeed(params.MoveSpeed / 10.f)
    {
        SetParams<MoveToParameters>(params);
    }
    
    void OnStart() override;
    NodeStatus Update() override;
    void OnFinished() override;
    void OnAbort() override;
private:
    bool m_bToPlayer;
    float m_StopDistance;
    float m_MoveSpeed;
    float m_DistanceToTarget;
};

struct MeleeEnemyAttackActionParameters : ParamsForAction
{
    HBlackboardKeyValue AttackPowerKey;
    float AttackDuration = 10.0f;
    void DrawImGui(HBlackboard* blackboard) override
    {
        DrawBlackboardIntKeySelector("Attack Power", AttackPowerKey, blackboard);
        DrawFloatValue("Attack Duration", AttackDuration);
    }
    void Serialize(YAML::Emitter& out) const override
    {
        SerializeBlackboardFloatKey("AttackPowerKey", AttackPowerKey, out);
        SerializeFloat("AttackDuration", AttackDuration, out);
    }
    void Deserialize(const YAML::Node& node) override
    {
        DeserializeBlackboardKey(node, "AttackPowerKey", AttackPowerKey);
        DeserializeFloat(node, "AttackDuration", AttackDuration);
    }
};
class MeleeEnemyAttackAction : public HActionNode
{
public:
    MeleeEnemyAttackAction(const std::string& name, const MeleeEnemyAttackActionParameters& params = MeleeEnemyAttackActionParameters{})
        : HActionNode(name, params), m_AttackPowerKey(params.AttackPowerKey), m_AttackDuration(params.AttackDuration)
    {
        SetParams<MeleeEnemyAttackActionParameters>(params);
    }

    void OnStart() override;
    NodeStatus Update() override;
    void OnFinished() override;
    void OnAbort() override;
private:
    HBlackboardKeyValue m_AttackPowerKey;
    float m_AttackDuration;
    float m_ElapsedTime = 0.0f;
};

struct HeavyAttackActionParameters : ParamsForAction
{
    HBlackboardKeyValue AttackPowerKey;
    HBlackboardKeyValue StaminaKey;
    float AttackDuration = 15.0f;
    float StaminaCost = 30.0f;
    void DrawImGui(HBlackboard* blackboard) override
    {
        DrawBlackboardIntKeySelector("Attack Power", AttackPowerKey, blackboard);
        DrawBlackboardFloatKeySelector("Stamina", StaminaKey, blackboard);
        DrawFloatValue("Attack Duration", AttackDuration);
        DrawFloatValue("Stamina Cost", StaminaCost);
    }
    void Serialize(YAML::Emitter& out) const override
    {
        SerializeBlackboardFloatKey("AttackPowerKey", AttackPowerKey, out);
        SerializeBlackboardFloatKey("StaminaKey", StaminaKey, out);
        SerializeFloat("AttackDuration", AttackDuration, out);
        SerializeFloat("StaminaCost", StaminaCost, out);
    }
    void Deserialize(const YAML::Node& node) override
    {
        DeserializeBlackboardKey(node, "AttackPowerKey", AttackPowerKey);
        DeserializeBlackboardKey(node, "StaminaKey", StaminaKey);
        DeserializeFloat(node, "AttackDuration", AttackDuration);
        DeserializeFloat(node, "StaminaCost", StaminaCost);
    }
};
class HeavyAttackAction : public HActionNode
{
public:
    HeavyAttackAction(const std::string& name, const HeavyAttackActionParameters& params = HeavyAttackActionParameters{})
        : HActionNode(name, params), m_AttackPowerKey(params.AttackPowerKey), m_StaminaKey(params.StaminaKey),
            m_AttackDuration(params.AttackDuration), m_StaminaCost(params.StaminaCost)
    {
        SetParams<HeavyAttackActionParameters>(params);
    }

    void OnStart() override;
    NodeStatus Update() override;
    void OnFinished() override;
    void OnAbort() override;
private:
    HBlackboardKeyValue m_AttackPowerKey;
    HBlackboardKeyValue m_StaminaKey;
    float m_AttackDuration;
    float m_StaminaCost;
    float m_ElapsedTime = 0.0f;
};