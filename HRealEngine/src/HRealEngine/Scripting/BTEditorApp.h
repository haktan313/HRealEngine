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

    private:
        std::unordered_map<int, MonoObject*> m_NodeToManagedActionParams;
        std::unordered_map<int, MonoObject*> m_NodeToManagedConditionParams;
        std::unordered_map<int, MonoObject*> m_NodeToManagedDecoratorParams;
    };
}