#pragma once
#include "ScriptEngine.h"

namespace HRealEngine
{
    class ScriptGlue
    {
    public:
        static void RegisterComponents();
        static void RegisterFunctions();
        static MonoObject* InstantiateClass(MonoClass* monoClass);
        static void NotifyBlackboardValuesChanged(HBlackboard& blackboard);
    };
}
