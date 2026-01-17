#pragma once

#include "BlackboardBase.h"
#include <string>
#include <yaml-cpp/yaml.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

enum class NodeStatus
{
    SUCCESS,
    FAILURE,
    RUNNING
};

enum class PriorityType
{
    None,
    Self,
    LowerPriority,
    Both
};

enum class HNodeType
{
    None,
    Action,
    Condition,
    Decorator,
    Composite,
    Root
};

typedef std::string HBlackboardKeyValue;

struct Params
{
    Params() = default;
    ~Params() = default;
    
    virtual void DrawImGui(HBlackboard* blackboard) {}
    void DrawFloatValue(const std::string& label, float& value)
    {
        ImGui::InputFloat(label.c_str(), &value);
    }
    void DrawIntValue(const std::string& label, int& value)
    {
        ImGui::InputInt(label.c_str(), &value);
    }
    void DrawBoolValue(const std::string& label, bool& value)
    {
        ImGui::Checkbox(label.c_str(), &value);
    }
    void DrawStringValue(const std::string& label, std::string& value)
    {
        char buffer[256];
        std::strncpy(buffer, value.c_str(), sizeof(buffer));
        if (ImGui::InputText(label.c_str(), buffer, sizeof(buffer)))
        {
            value = std::string(buffer);
        }
    }
    void DrawBlackboardFloatKeySelector(const std::string& label, std::string& key, HBlackboard* blackboard)
    {
        const char* preview = key.empty() ? "Select float key..." : key.c_str();
        if (ImGui::BeginCombo(label.c_str(), preview))
        {
            if (blackboard)
                for (const auto& [bbKey, value] : blackboard->GetFloatValues())
                {
                    bool isSelected = (key == bbKey);
                    if (ImGui::Selectable(bbKey.c_str(), isSelected))
                        key = bbKey;
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
            ImGui::EndCombo();
        }
    }
    void DrawBlackboardIntKeySelector(const std::string& label, std::string& key, HBlackboard* blackboard)
    {
        const char* preview = key.empty() ? "Select int key..." : key.c_str();
        if (ImGui::BeginCombo(label.c_str(), preview))
        {
            if (blackboard)
                for (const auto& [bbKey, value] : blackboard->GetIntValues())
                {
                    bool isSelected = (key == bbKey);
                    if (ImGui::Selectable(bbKey.c_str(), isSelected))
                        key = bbKey;
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
            ImGui::EndCombo();
        }
    }
    void DrawBlackboardBoolKeySelector(const std::string& label, std::string& key, HBlackboard* blackboard)
    {
        const char* preview = key.empty() ? "Select bool key..." : key.c_str();
        if (ImGui::BeginCombo(label.c_str(), preview))
        {
            if (blackboard)
                for (const auto& [bbKey, value] : blackboard->GetBoolValues())
                {
                    bool isSelected = (key == bbKey);
                    if (ImGui::Selectable(bbKey.c_str(), isSelected))
                        key = bbKey;
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
            ImGui::EndCombo();
        }
    }
    void DrawBlackboardStringKeySelector(const std::string& label, std::string& key, HBlackboard* blackboard)
    {
        const char* preview = key.empty() ? "Select string key..." : key.c_str();
        if (ImGui::BeginCombo(label.c_str(), preview))
        {
            if (blackboard)
                for (const auto& [bbKey, value] : blackboard->GetStringValues())
                {
                    bool isSelected = (key == bbKey);
                    if (ImGui::Selectable(bbKey.c_str(), isSelected))
                        key = bbKey;
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
            ImGui::EndCombo();
        }
    }
    
    virtual void Serialize(YAML::Emitter& out) const {}
    virtual void Deserialize(const YAML::Node& node) {}
    
    void SerializeBool(const std::string& name, bool value, YAML::Emitter& out) const
    {
        out << YAML::Key << name << YAML::Value << value;
    }
    void SerializeInt(const std::string& name, int value, YAML::Emitter& out) const
    {
        out << YAML::Key << name << YAML::Value << value;
    }
    void SerializeFloat(const std::string& name, float value, YAML::Emitter& out) const
    {
        out << YAML::Key << name << YAML::Value << value;
    }
    void SerializeString(const std::string& name, const std::string& value, YAML::Emitter& out) const
    {
        out << YAML::Key << name << YAML::Value << value;
    }
    
    void SerializeBlackboardFloatKey(const std::string& name, const HBlackboardKeyValue& key, YAML::Emitter& out) const 
    {
        out << YAML::Key << name << YAML::Value << key;
    }
    void SerializeBlackboardIntKey(const std::string& name, const HBlackboardKeyValue& key, YAML::Emitter& out) const 
    {
        out << YAML::Key << name << YAML::Value << key;
    }
    void SerializeBlackboardBoolKey(const std::string& name, const HBlackboardKeyValue& key, YAML::Emitter& out) const 
    {
        out << YAML::Key << name << YAML::Value << key;
    }
    void SerializeBlackboardStringKey(const std::string& name, const HBlackboardKeyValue& key, YAML::Emitter& out) const 
    {
        out << YAML::Key << name << YAML::Value << key;
    }
    
    void DeserializeBool(const YAML::Node& node, const std::string& name, bool& value)
    {
        if (node[name])
            value = node[name].as<bool>();
    }
    void DeserializeInt(const YAML::Node& node, const std::string& name, int& value)
    {
        if (node[name])
            value = node[name].as<int>();
    }
    void DeserializeFloat(const YAML::Node& node, const std::string& name, float& value)
    {
        if (node[name])
            value = node[name].as<float>();
    }
    void DeserializeString(const YAML::Node& node, const std::string& name, std::string& value)
    {
        if (node[name])
            value = node[name].as<std::string>();
    }
    void DeserializeBlackboardKey(const YAML::Node& node, const std::string& name, HBlackboardKeyValue& key)
    {
        if (node[name])
            key = node[name].as<std::string>();
    }
};