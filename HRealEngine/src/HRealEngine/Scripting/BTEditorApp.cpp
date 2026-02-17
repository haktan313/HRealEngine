#include "HRpch.h"
#include "BTEditorApp.h"

namespace HRealEngine
{
    void BTEditorApp::DrawActionNodeParameters(int nodeKey, const std::string& classId)
    {
        ImGui::Text("Parameters");
        ImGui::Separator();

        if (classId.empty())
            return;
        
        bool isManagedAction = false;
        if (ScriptEngine::IsInitialized())
        {
            auto& managedActions = ScriptEngine::s_BTActionClasses;
            if (managedActions.find(classId) != managedActions.end())
                isManagedAction = true;
        }

        if (isManagedAction)
        {
            auto it = m_NodeToManagedActionParams.find(nodeKey);
            if (it == m_NodeToManagedActionParams.end())
            {
                MonoObject* paramsInstance = ScriptEngine::CreateBTParameterInstance(classId);
                if (paramsInstance)
                {
                    m_NodeToManagedActionParams[nodeKey] = paramsInstance;
                    LOG_CORE_INFO("Created managed parameter instance for {}", classId);
                }
                it = m_NodeToManagedActionParams.find(nodeKey);
            }

            if (it != m_NodeToManagedActionParams.end() && it->second)
            {
                LOG_CORE_INFO("Drawing C# parameters for {}", classId);
                ScriptEngine::DrawBTParametersImGui(it->second, m_Blackboard.get());
            }
            else
            {
                LOG_CORE_WARN("No managed parameters found for {}", classId);
            }
        }
        else
        {
            auto parameter = m_NodeToParams.find(nodeKey);
            if (parameter != m_NodeToParams.end() && parameter->second)
                parameter->second->DrawImGui(m_Blackboard.get());
        }
    }

    void BTEditorApp::DrawDecoratorNodeParameters(int nodeKey, const std::string& classId)
    {
        ImGui::Text("Parameters");
        ImGui::Separator();

        if (classId.empty())
            return;

        bool isManagedDecorator = false;
        if (ScriptEngine::IsInitialized())
        {
            auto& managedDecorators = ScriptEngine::s_BTDecoratorClasses;
            if (managedDecorators.find(classId) != managedDecorators.end())
                isManagedDecorator = true;
        }

        if (isManagedDecorator)
        {
            auto it = m_NodeToManagedDecoratorParams.find(nodeKey);
            if (it == m_NodeToManagedDecoratorParams.end())
            {
                m_NodeToManagedDecoratorParams[nodeKey] = ScriptEngine::CreateBTParameterInstance(classId);
                it = m_NodeToManagedDecoratorParams.find(nodeKey);
            }

            if (it != m_NodeToManagedDecoratorParams.end() && it->second)
                ScriptEngine::DrawBTParametersImGui(it->second, m_Blackboard.get());
        }
        else
        {
            auto parameter = m_NodeToDecoratorParams.find(nodeKey);
            if (parameter != m_NodeToDecoratorParams.end() && parameter->second)
                parameter->second->DrawImGui(m_Blackboard.get());
        }
    }

    void BTEditorApp::DrawConditionNodeParameters(int nodeKey, const std::string& classId)
    {
        ImGui::Text("Parameters");
        ImGui::Separator();

        if (classId.empty())
            return;

        bool isManagedCondition = false;
        if (ScriptEngine::IsInitialized())
        {
            auto& managedConditions = ScriptEngine::s_BTConditionClasses;
            if (managedConditions.find(classId) != managedConditions.end())
                isManagedCondition = true;
        }

        if (isManagedCondition)
        {
            auto it = m_NodeToManagedConditionParams.find(nodeKey);
            if (it == m_NodeToManagedConditionParams.end())
            {
                m_NodeToManagedConditionParams[nodeKey] = ScriptEngine::CreateBTParameterInstance(classId);
                it = m_NodeToManagedConditionParams.find(nodeKey);
            }

            if (it != m_NodeToManagedConditionParams.end() && it->second)
            {
                ImGui::Text("Priority Type");
                ImGui::Separator();
                const char* priorityTypes[] = { "None", "Self", "Lower Priority", "Both" };
                
                auto parameter = m_NodeToConditionParams.find(nodeKey);
                if (parameter != m_NodeToConditionParams.end() && parameter->second)
                {
                    int currentPriority = static_cast<int>(parameter->second->Priority);
                    if (ImGui::Combo("##PriorityTypeCombo", &currentPriority, priorityTypes, IM_ARRAYSIZE(priorityTypes)))
                        parameter->second->Priority = static_cast<PriorityType>(currentPriority);
                }
                
                ScriptEngine::DrawBTParametersImGui(it->second, m_Blackboard.get());
            }
        }
        else
        {
            auto parameter = m_NodeToConditionParams.find(nodeKey);
            if (parameter != m_NodeToConditionParams.end() && parameter->second)
            {
                ImGui::Text("Priority Type");
                ImGui::Separator();
                const char* priorityTypes[] = { "None", "Self", "Lower Priority", "Both" };
                int currentPriority = static_cast<int>(parameter->second->Priority);
                if (ImGui::Combo("##PriorityTypeCombo", &currentPriority, priorityTypes, IM_ARRAYSIZE(priorityTypes)))
                    parameter->second->Priority = static_cast<PriorityType>(currentPriority);
                
                parameter->second->DrawImGui(m_Blackboard.get());
            }
        }
    }
}