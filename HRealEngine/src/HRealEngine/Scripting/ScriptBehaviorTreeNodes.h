#pragma once

#include "BehaviorTreeThings/Core/Nodes.h"

struct _MonoMethod;
struct _MonoObject;

typedef struct _MonoMethod MonoMethod;
typedef struct _MonoObject MonoObject;

namespace HRealEngine
{
    void RegisterScriptBehaviorTreeAction(const std::string& displayName, const std::string& managedTypeName);
    void RegisterScriptBehaviorTreeCondition(const std::string& displayName, const std::string& managedTypeName);
    void RegisterScriptBehaviorTreeDecorator(const std::string& displayName, const std::string& managedTypeName);
    void RegisterScriptBehaviorTreeBlackboard(const std::string& displayName, const std::string& managedTypeName);

    bool CreateScriptBlackboardBool(const std::string& key, bool value);
    bool CreateScriptBlackboardInt(const std::string& key, int value);
    bool CreateScriptBlackboardFloat(const std::string& key, float value);
    bool CreateScriptBlackboardString(const std::string& key, const std::string& value);

    uint64_t GetCurrentBehaviorTreeOwnerEntityID();
}
