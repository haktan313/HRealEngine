#include "HRpch.h"
#include "BehaviorTreeThings/Editor/NodeEditorStructsAndEnums.h" 
#include "CSharpNodeRegistry.h"
#include "HRealEngine/Scripting/ScriptEngine.h"
#include "ManagedBTNodes.h"

namespace HRealEngine
{
    void CSharpNodeRegistry::AddManagedActionNode(const std::string& className)
    {
        ActionClassInfo actionInfo;
        actionInfo.Name = className;
        
        actionInfo.CreateParamsFn = [className]() -> std::unique_ptr<ParamsForAction>
        {
            return std::make_unique<ParamsForAction>();
        };

        actionInfo.BuildFn = [className](BehaviorTreeBuilder& builder, Node* node, ParamsForAction& baseParams)
        {
            MonoObject* paramsInstance = HRealEngine::ScriptEngine::CreateBTParameterInstance(className);
            
            builder.action<HRealEngine::ManagedBTAction>(node->Name, className, paramsInstance);
        };

        actionInfo.BuildFromYAML = [className](BehaviorTreeBuilder& builder, const std::string& instanceName, const YAML::Node& paramsNode)
        {
            MonoObject* paramsInstance = HRealEngine::ScriptEngine::CreateBTParameterInstance(className);
            
            if (paramsInstance && paramsNode)
            {
                HRealEngine::ScriptEngine::DeserializeBTParameters(paramsInstance, paramsNode);
            }
            
            builder.action<HRealEngine::ManagedBTAction>(instanceName, className, paramsInstance);
        };
        
        s_ActionClassInfoMap.emplace(className, std::move(actionInfo));
    }

    void CSharpNodeRegistry::AddManagedConditionNode(const std::string& className)
    {
        ConditionClassInfo conditionInfo;
        conditionInfo.Name = className;
        
        conditionInfo.CreateParamsFn = [className]() -> std::unique_ptr<ParamsForCondition>
        {
            return std::make_unique<ParamsForCondition>();
        };

        conditionInfo.BuildFn = [className](BehaviorTreeBuilder& builder, ParamsForCondition& baseParams)
        {
            MonoObject* paramsInstance = HRealEngine::ScriptEngine::CreateBTParameterInstance(className);
            
            builder.condition<HRealEngine::ManagedBTCondition>(baseParams.Priority, className, className, paramsInstance);
        };

        conditionInfo.BuildFromYAML = [className](BehaviorTreeBuilder& builder, const std::string& instanceName, const YAML::Node& paramsNode, PriorityType priority, bool alwaysReevaluate)
        {
            MonoObject* paramsInstance = HRealEngine::ScriptEngine::CreateBTParameterInstance(className);
            
            if (paramsInstance && paramsNode)
            {
                HRealEngine::ScriptEngine::DeserializeBTParameters(paramsInstance, paramsNode);
            }
            
            builder.condition<HRealEngine::ManagedBTCondition>(priority, instanceName, className, paramsInstance);
            builder.setLastConditionAlwaysReevaluate(alwaysReevaluate);
        };
        
        s_ConditionClassInfoMap.emplace(className, std::move(conditionInfo));
    }

    void CSharpNodeRegistry::AddManagedDecoratorNode(const std::string& className)
    {
        DecoratorClassInfo decoratorInfo;
        decoratorInfo.Name = className;
        
        decoratorInfo.CreateParamsFn = [className]() -> std::unique_ptr<ParamsForDecorator>
        {
            return std::make_unique<ParamsForDecorator>();
        };

        decoratorInfo.BuildFn = [className](BehaviorTreeBuilder& builder, ParamsForDecorator& baseParams)
        {
            MonoObject* paramsInstance = HRealEngine::ScriptEngine::CreateBTParameterInstance(className);
            
            builder.decorator<HRealEngine::ManagedBTDecorator>(className, className, paramsInstance);
        };

        decoratorInfo.BuildFromYAML = [className](BehaviorTreeBuilder& builder, const std::string& instanceName, const YAML::Node& paramsNode)
        {
            MonoObject* paramsInstance = HRealEngine::ScriptEngine::CreateBTParameterInstance(className);
            
            if (paramsInstance && paramsNode)
            {
                HRealEngine::ScriptEngine::DeserializeBTParameters(paramsInstance, paramsNode);
            }
            
            builder.decorator<HRealEngine::ManagedBTDecorator>(instanceName, className, paramsInstance);
        };
        
        s_DecoratorClassInfoMap.emplace(className, std::move(decoratorInfo));
    }

    void CSharpNodeRegistry::AddManagedBlackboard(const std::string& className)
    {
        BlackboardClassInfo blackboardInfo;
        blackboardInfo.Name = className;
        
        blackboardInfo.CreateBlackboardFn = [className]() -> std::unique_ptr<HBlackboard>
        {
            return std::make_unique<HRealEngine::ManagedBTBlackboard>(className, className);
        };
        
        s_BlackboardClassInfoMap.emplace(className, std::move(blackboardInfo));
    }
}
