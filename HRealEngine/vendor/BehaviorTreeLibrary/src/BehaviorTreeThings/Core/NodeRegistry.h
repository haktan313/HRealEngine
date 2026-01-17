#pragma once
#include <string>
#include "Tree.h"
#include "Editor/NodeEditorStructsAndEnums.h"

class NodeRegistry
{
public:
    template<typename ActionClass, typename ParamsStruct>
    static void AddActionNodeToBuilder(const std::string& name = "")
    {
        ActionClassInfo actionInfo;
        actionInfo.Name = name;
        actionInfo.CreateParamsFn = []()
        {
            return std::make_unique<ParamsStruct>();
        };

        actionInfo.BuildFn = [](BehaviorTreeBuilder& builder, Node* node, ParamsForAction& baseParams)
        {
            auto& params = static_cast<ParamsStruct&>(baseParams);
            builder.action<ActionClass>(node->Name, params);
        };
        actionInfo.BuildFromYAML = [](BehaviorTreeBuilder& builder, const std::string& instanceName, const YAML::Node& paramsNode)
        {
            ParamsStruct params;
            params.Deserialize(paramsNode);
            builder.action<ActionClass>(instanceName, params);
        };
        s_ActionClassInfoMap.emplace(name, std::move(actionInfo));
    }
    template<typename DecoratorClass, typename ParamsStruct>
    static void AddDecoratorNodeToBuilder(const std::string& name = "")
    {
        DecoratorClassInfo decoratorInfo;
        decoratorInfo.Name = name;
        decoratorInfo.CreateParamsFn = []()
        {
            return std::make_unique<ParamsStruct>();
        };
        decoratorInfo.BuildFn = [name](BehaviorTreeBuilder& builder, ParamsForDecorator& baseParams)
        {
            auto& params = static_cast<ParamsStruct&>(baseParams);
            builder.decorator<DecoratorClass>(name, params);
        };
        decoratorInfo.BuildFromYAML = [](BehaviorTreeBuilder& builder, const std::string& instanceName, const YAML::Node& paramsNode)
        {
            ParamsStruct p;
            p.Deserialize(paramsNode);
            builder.decorator<DecoratorClass>(instanceName, p);
        };
        s_DecoratorClassInfoMap.emplace(name, std::move(decoratorInfo));
    }
    template<typename ConditionClass, typename ParamsStruct>
    static void AddConditionNodeToBuilder(const std::string& name = "")
    {
        ConditionClassInfo conditionInfo;
        conditionInfo.Name = name;
        conditionInfo.CreateParamsFn = []()
        {
            return std::make_unique<ParamsStruct>();
        };
        conditionInfo.BuildFn = [name](BehaviorTreeBuilder& builder, ParamsForCondition& baseParams)
        {
            auto& params = static_cast<ParamsStruct&>(baseParams);
            builder.condition<ConditionClass>(baseParams.Priority, name, params);
        };
        conditionInfo.BuildFromYAML = [](BehaviorTreeBuilder& builder, const std::string& instanceName, const YAML::Node& paramsNode, PriorityType priority)
        {
            ParamsStruct p;
            p.Deserialize(paramsNode);
            builder.condition<ConditionClass>(priority, instanceName, p);
        };
        s_ConditionClassInfoMap.emplace(name, std::move(conditionInfo));
    }
    template<typename BlackboardType>
    static void AddBlackBoardToEditor(const std::string& name = "")
    {
        BlackboardClassInfo blackboardInfo;
        blackboardInfo.Name = name;
        blackboardInfo.CreateBlackboardFn = [name]()
        {
            return std::make_unique<BlackboardType>(name);
        };
        s_BlackboardClassInfoMap.emplace(name, std::move(blackboardInfo));
    }

    static std::unordered_map<std::string, ActionClassInfo>& GetActionClassInfoMap() { return s_ActionClassInfoMap; }
    static std::unordered_map<std::string, DecoratorClassInfo>& GetDecoratorClassInfoMap() { return s_DecoratorClassInfoMap; }
    static std::unordered_map<std::string, ConditionClassInfo>& GetConditionClassInfoMap() { return s_ConditionClassInfoMap; }
    static std::unordered_map<std::string, BlackboardClassInfo>& GetBlackboardClassInfoMap() { return s_BlackboardClassInfoMap; }
private:
    static std::unordered_map<std::string, ActionClassInfo> s_ActionClassInfoMap;
    static std::unordered_map<std::string, DecoratorClassInfo> s_DecoratorClassInfoMap;
    static std::unordered_map<std::string, ConditionClassInfo> s_ConditionClassInfoMap;
    static std::unordered_map<std::string, BlackboardClassInfo> s_BlackboardClassInfoMap;
};
