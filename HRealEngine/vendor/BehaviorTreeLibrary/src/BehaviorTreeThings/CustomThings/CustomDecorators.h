#pragma once
#include "imgui.h"
#include "Nodes.h"

struct ChangeResultOfTheNodeParameters : ParamsForDecorator
{
    NodeStatus NewResult = NodeStatus::SUCCESS;
    void DrawImGui(HBlackboard* blackboard) override
    {
        const char* items[] = { "SUCCESS", "FAILURE", "RUNNING" };
        int currentItem = static_cast<int>(NewResult);
        if (ImGui::Combo("New Result", &currentItem, items, IM_ARRAYSIZE(items)))
        {
            NewResult = static_cast<NodeStatus>(currentItem);
        }
    }
    void Serialize(YAML::Emitter& out) const override
    {
        SerializeInt("NewResult", static_cast<int>(NewResult), out);
    }
    void Deserialize(const YAML::Node& node) override
    {
        int resultInt = 0;
        DeserializeInt(node, "NewResult", resultInt);
        NewResult = static_cast<NodeStatus>(resultInt);
    }
};
class ChangeResultOfTheNodeDecorator : public HDecorator
{
public:
    ChangeResultOfTheNodeDecorator(const std::string& name, const ChangeResultOfTheNodeParameters& params = ChangeResultOfTheNodeParameters{})
        : HDecorator(name, params), m_NewResult(params.NewResult)
    {
        SetParams<ChangeResultOfTheNodeParameters>(params);
    }
    void OnStart() override;
    bool CanExecute() override;
    void OnFinishedResult(NodeStatus& status) override;
    void OnFinished() override;
    void OnAbort() override;
private:
    NodeStatus m_NewResult;
};

struct CooldownDecoratorParameters : ParamsForDecorator
{
    float CooldownTime = 5.0f;
    void DrawImGui(HBlackboard* blackboard) override
    {
        DrawFloatValue("Cooldown Time", CooldownTime);
    }
    void Serialize(YAML::Emitter& out) const override
    {
        SerializeFloat("CooldownTime", CooldownTime, out);
    }
    void Deserialize(const YAML::Node& node) override
    {
        DeserializeFloat(node, "CooldownTime", CooldownTime);
    }
};
class CooldownDecorator : public HDecorator
{
public:
    CooldownDecorator(const std::string& name, const CooldownDecoratorParameters& params = CooldownDecoratorParameters{})
        : HDecorator(name, params), m_CooldownTime(params.CooldownTime), m_LastExecutionTime(-params.CooldownTime)
    {
        SetParams<CooldownDecoratorParameters>(params);
    }
    void OnStart() override;
    bool CanExecute() override;
    void OnFinishedResult(NodeStatus& status) override;
    void OnFinished() override;
    void OnAbort() override;
private:
    float m_CooldownTime;
    float m_LastExecutionTime;
};