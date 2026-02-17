#pragma once
#include "BehaviorTreeThings/Core/NodeRegistry.h"

namespace HRealEngine
{
    class CSharpNodeRegistry : public NodeRegistry
    {
    public:
        static void AddManagedActionNode(const std::string& className);
        static void AddManagedConditionNode(const std::string& className);
        static void AddManagedDecoratorNode(const std::string& className);
        static void AddManagedBlackboard(const std::string& className);
    };
}
