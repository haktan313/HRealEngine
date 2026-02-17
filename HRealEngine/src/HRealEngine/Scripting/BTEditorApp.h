#pragma once
#include "BehaviorTreeThings/Editor/NodeEditorApp.h"
#include "HRealEngine/Scripting/ScriptEngine.h"

namespace HRealEngine
{
    class BTEditorApp : public NodeEditorApp
    {
    public:
        BTEditorApp() = default;
        ~BTEditorApp() = default;
        
        void DrawActionNodeParameters(int nodeKey, const std::string& classId) override;
        void DrawDecoratorNodeParameters(int nodeKey, const std::string& classId) override;
        void DrawConditionNodeParameters(int nodeKey, const std::string& classId) override;

        void* GetManagedActionParams(int nodeKey) const override
        {
            auto it = m_NodeToManagedActionParams.find(nodeKey);
            return (it != m_NodeToManagedActionParams.end()) ? it->second : nullptr;
        }
        void* GetManagedDecoratorParams(int nodeKey) const override
        {
            auto it = m_NodeToManagedDecoratorParams.find(nodeKey);
            return (it != m_NodeToManagedDecoratorParams.end()) ? it->second : nullptr;
        }
        void* GetManagedConditionParams(int nodeKey) const override
        {
            auto it = m_NodeToManagedConditionParams.find(nodeKey);
            return (it != m_NodeToManagedConditionParams.end()) ? it->second : nullptr;
        }

        void DeserializeManagedActionParams(int nodeKey, const std::string& className, const YAML::Node& paramsNode) override;
        void DeserializeManagedDecoratorParams(int nodeKey, const std::string& className, const YAML::Node& paramsNode) override;
        void DeserializeManagedConditionParams(int nodeKey, const std::string& className, const YAML::Node& paramsNode) override;
    private:
        std::unordered_map<int, MonoObject*> m_NodeToManagedActionParams;
        std::unordered_map<int, MonoObject*> m_NodeToManagedConditionParams;
        std::unordered_map<int, MonoObject*> m_NodeToManagedDecoratorParams;
    };
}