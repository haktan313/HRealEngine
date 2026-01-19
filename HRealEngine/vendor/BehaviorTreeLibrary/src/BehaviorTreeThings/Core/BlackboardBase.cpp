#include "BlackboardBase.h"
#include "imgui.h"

bool HBlackboard::GetBoolValue(const std::string& key) const
{
    if (m_BoolValues.find(key) != m_BoolValues.end())
        return m_BoolValues.at(key);
    return false;
}

int HBlackboard::GetIntValue(const std::string& key) const
{
    if (m_IntValues.find(key) != m_IntValues.end())
        return m_IntValues.at(key);
    return 0;
}

float HBlackboard::GetFloatValue(const std::string& key) const
{
    if (m_FloatValues.find(key) != m_FloatValues.end())
        return m_FloatValues.at(key);
    return 0.0f;
}

const std::string& HBlackboard::GetStringValue(const std::string& key) const
{
    if (m_StringValues.find(key) != m_StringValues.end())
        return m_StringValues.at(key);
    return "";
}

void HBlackboard::SetBoolValue(const std::string& key, bool value)
{
    if (m_BoolValues.find(key) != m_BoolValues.end())
        m_BoolValues[key] = value;
    m_bValuesChanged = true;
}

void HBlackboard::SetIntValue(const std::string& key, int value)
{
    if (m_IntValues.find(key) != m_IntValues.end())
        m_IntValues[key] = value;
    m_bValuesChanged = true;
}

void HBlackboard::SetFloatValue(const std::string& key, float value)
{
    if (m_FloatValues.find(key) != m_FloatValues.end())
        m_FloatValues[key] = value;
    m_bValuesChanged = true;
}

void HBlackboard::SetStringValue(const std::string& key, const std::string& value)
{
    if (m_StringValues.find(key) != m_StringValues.end())
        m_StringValues[key] = value;
    m_bValuesChanged = true;
}

void HBlackboard::DrawImGui()
{
    ImGui::Text("Blackboard Values:");
    ImGui::Separator();
    for (auto& [key, value] : m_BoolValues)
    {
        bool val = value;
        if (ImGui::Checkbox(key.c_str(), &val))
            m_BoolValues[key] = val;
    }
    for (auto& [key, value] : m_IntValues)
    {
        int val = value;
        if (ImGui::InputInt(key.c_str(), &val))
            m_IntValues[key] = val;
    }
    for (auto& [key, value] : m_FloatValues)
    {
        float val = value;
        if (ImGui::InputFloat(key.c_str(), &val))
            m_FloatValues[key] = val;
    }
    for (auto& [key, value] : m_StringValues)
    {
        char buffer[256];
        std::strncpy(buffer, value.c_str(), sizeof(buffer));
        if (ImGui::InputText(key.c_str(), buffer, sizeof(buffer)))
            m_StringValues[key] = std::string(buffer);
    }
}

void HBlackboard::CreateBoolValue(const std::string& key, bool value)
{
    m_BoolValues[key] = value;
}

void HBlackboard::CreateIntValue(const std::string& key, int value)
{
    m_IntValues[key] = value;
}

void HBlackboard::CreateFloatValue(const std::string& key, float value)
{
    m_FloatValues[key] = value;
}

void HBlackboard::CreateStringValue(const std::string& key, const std::string& value)
{
    m_StringValues[key] = value;
}
