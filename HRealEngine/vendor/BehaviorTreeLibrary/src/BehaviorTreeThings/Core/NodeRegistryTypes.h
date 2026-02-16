#pragma once

#include <functional>
#include <memory>
#include <string>

#include "Nodes.h"

struct Node;

struct ActionClassInfo
{
    std::string Name;
    std::function<void(BehaviorTreeBuilder&, Node*, ParamsForAction&)> BuildFn;
    std::function<std::unique_ptr<ParamsForAction>()> CreateParamsFn;
    std::function<void(BehaviorTreeBuilder&, const std::string&, const YAML::Node&)> BuildFromYAML;
};

struct DecoratorClassInfo
{
    std::string Name;
    std::function<void(BehaviorTreeBuilder&, ParamsForDecorator&)> BuildFn;
    std::function<std::unique_ptr<ParamsForDecorator>()> CreateParamsFn;
    std::function<void(BehaviorTreeBuilder&, const std::string&, const YAML::Node&)> BuildFromYAML;
};

struct ConditionClassInfo
{
    std::string Name;
    std::function<void(BehaviorTreeBuilder&, ParamsForCondition&)> BuildFn;
    std::function<std::unique_ptr<ParamsForCondition>()> CreateParamsFn;
    std::function<void(BehaviorTreeBuilder&, const std::string&, const YAML::Node&, PriorityType)> BuildFromYAML;
};

struct BlackboardClassInfo
{
    std::string Name;
    std::function<std::unique_ptr<HBlackboard>()> CreateBlackboardFn;
};
