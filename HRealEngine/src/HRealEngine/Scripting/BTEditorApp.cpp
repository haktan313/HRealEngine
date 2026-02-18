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
            auto classNameIt = m_NodeToManagedActionClassName.find(nodeKey);
            std::string lastClassName = (classNameIt != m_NodeToManagedActionClassName.end()) 
                ? classNameIt->second : "";

            if (lastClassName != classId)
            {
                LOG_CORE_INFO("Action class changed from '{}' to '{}', creating new parameter instance", 
                    lastClassName, classId);
                
                MonoObject* paramsInstance = ScriptEngine::CreateBTParameterInstance(classId);
                if (paramsInstance)
                {
                    m_NodeToManagedActionParams[nodeKey] = paramsInstance;
                    m_NodeToManagedActionClassName[nodeKey] = classId;
                }
            }

            auto it = m_NodeToManagedActionParams.find(nodeKey);
            if (it != m_NodeToManagedActionParams.end() && it->second)
            {
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
            auto classNameIt = m_NodeToManagedDecoratorClassName.find(nodeKey);
            std::string lastClassName = (classNameIt != m_NodeToManagedDecoratorClassName.end()) 
                ? classNameIt->second : "";
            
            if (lastClassName != classId)
            {
                LOG_CORE_INFO("Decorator class changed from '{}' to '{}', creating new parameter instance", 
                    lastClassName, classId);
                
                MonoObject* paramsInstance = ScriptEngine::CreateBTParameterInstance(classId);
                if (paramsInstance)
                {
                    m_NodeToManagedDecoratorParams[nodeKey] = paramsInstance;
                    m_NodeToManagedDecoratorClassName[nodeKey] = classId;
                }
            }

            auto it = m_NodeToManagedDecoratorParams.find(nodeKey);
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
            auto classNameIt = m_NodeToManagedConditionClassName.find(nodeKey);
            std::string lastClassName = (classNameIt != m_NodeToManagedConditionClassName.end()) 
                ? classNameIt->second : "";
            
            if (lastClassName != classId)
            {
                LOG_CORE_INFO("Condition class changed from '{}' to '{}', creating new parameter instance", 
                    lastClassName, classId);
                
                MonoObject* paramsInstance = ScriptEngine::CreateBTParameterInstance(classId);
                if (paramsInstance)
                {
                    m_NodeToManagedConditionParams[nodeKey] = paramsInstance;
                    m_NodeToManagedConditionClassName[nodeKey] = classId;
                }
            }

            auto it = m_NodeToManagedConditionParams.find(nodeKey);
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
    void BTEditorApp::DeserializeManagedActionParams(int nodeKey, const std::string& className, const YAML::Node& paramsNode)
    {
        if (!ScriptEngine::IsInitialized())
            return;
        if (ScriptEngine::s_BTActionClasses.find(className) == ScriptEngine::s_BTActionClasses.end())
            return;

        MonoObject* paramsInstance = ScriptEngine::CreateBTParameterInstance(className);
        if (!paramsInstance)
            return;

        ScriptEngine::DeserializeBTParameters(paramsInstance, paramsNode);
        m_NodeToManagedActionParams[nodeKey] = paramsInstance;
        m_NodeToActionClassId[nodeKey] = className;
    }

    void BTEditorApp::DeserializeManagedDecoratorParams(int nodeKey, const std::string& className, const YAML::Node& paramsNode)
    {
        if (!ScriptEngine::IsInitialized())
            return;
        if (ScriptEngine::s_BTDecoratorClasses.find(className) == ScriptEngine::s_BTDecoratorClasses.end())
            return;

        MonoObject* paramsInstance = ScriptEngine::CreateBTParameterInstance(className);
        if (!paramsInstance)
            return;

        ScriptEngine::DeserializeBTParameters(paramsInstance, paramsNode);
        m_NodeToManagedDecoratorParams[nodeKey] = paramsInstance;
        m_NodeToDecoratorClassId[nodeKey] = className;
    }

    void BTEditorApp::DeserializeManagedConditionParams(int nodeKey, const std::string& className, const YAML::Node& paramsNode)
    {
        if (!ScriptEngine::IsInitialized())
            return;
        if (ScriptEngine::s_BTConditionClasses.find(className) == ScriptEngine::s_BTConditionClasses.end())
            return;

        MonoObject* paramsInstance = ScriptEngine::CreateBTParameterInstance(className);
        if (!paramsInstance)
            return;

        ScriptEngine::DeserializeBTParameters(paramsInstance, paramsNode);
        m_NodeToManagedConditionParams[nodeKey] = paramsInstance;
        m_NodeToConditionClassId[nodeKey] = className;
    }
}
