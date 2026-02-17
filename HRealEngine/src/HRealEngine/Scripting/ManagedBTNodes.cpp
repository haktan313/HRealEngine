#include "HRpch.h"
#include "ManagedBTNodes.h"
#include "HRealEngine/Scripting/ScriptEngine.h"
#include "HRealEngine/Core/Entity.h"

namespace HRealEngine
{
    //---------------------BTBlackboard---------------------
    
    ManagedBTBlackboard::ManagedBTBlackboard(const std::string& name, const std::string& managedClassName)
        : HBlackboard(name), m_ManagedClassName(managedClassName)
    {
        m_ManagedInstance = ScriptEngine::CreateBTBlackboardInstance(managedClassName);
        
        if (m_ManagedInstance)
        {
            SyncFromManagedBlackboard();
        }
    }

    ManagedBTBlackboard::~ManagedBTBlackboard()
    {

    }

    void ManagedBTBlackboard::SyncFromManagedBlackboard()
    {
        MonoClass* klass = mono_object_get_class(m_ManagedInstance);
        MonoDomain* domain = mono_object_get_domain(m_ManagedInstance);
        
        auto findMethod = [](MonoClass* k, const char* name) -> MonoMethod*
        {
            MonoClass* current = k;
            while (current != nullptr)
            {
                MonoMethod* method = mono_class_get_method_from_name(current, name, 0);
                if (method)
                    return method;
                current = mono_class_get_parent(current);
            }
            return nullptr;
        };

        auto getStringArray = [&](const char* methodName) -> std::vector<std::string>
        {
            std::vector<std::string> result;
            MonoMethod* method = findMethod(klass, methodName);
            if (!method)
            {
                LOG_CORE_WARN("SyncFromManagedBlackboard: Method '{}' not found!", methodName);
                return result;
            }
            MonoArray* arr = (MonoArray*)mono_runtime_invoke(method, m_ManagedInstance, nullptr, nullptr);
            if (!arr)
                return result;
            int len = (int)mono_array_length(arr);
            for (int i = 0; i < len; i++)
            {
                MonoString* s = mono_array_get(arr, MonoString*, i);
                char* c = s ? mono_string_to_utf8(s) : nullptr;
                result.push_back(c ? c : "");
                if (c)
                    mono_free(c);
            }
            return result;
        };
        
        // Bool
        {
            auto keys = getStringArray("GetBoolKeys");
            MonoMethod* valsMethod = findMethod(klass, "GetBoolVals");
            if (valsMethod && !keys.empty())
            {
                MonoArray* valsArr = (MonoArray*)mono_runtime_invoke(valsMethod, m_ManagedInstance, nullptr, nullptr);
                if (valsArr)
                    for (int i = 0; i < (int)keys.size(); i++)
                    {
                        bool val = mono_array_get(valsArr, MonoBoolean, i) != 0;
                        CreateBoolValue(keys[i], val);
                    }
            }
        }
        // Int
        {
            auto keys = getStringArray("GetIntKeys");
            MonoMethod* valsMethod = findMethod(klass, "GetIntVals");
            if (valsMethod && !keys.empty())
            {
                MonoArray* valsArr = (MonoArray*)mono_runtime_invoke(valsMethod, m_ManagedInstance, nullptr, nullptr);
                if (valsArr)
                    for (int i = 0; i < (int)keys.size(); i++)
                    {
                        int val = mono_array_get(valsArr, int, i);
                        CreateIntValue(keys[i], val);
                    }
            }
        }
        // Float
        {
            auto keys = getStringArray("GetFloatKeys");
            MonoMethod* valsMethod = findMethod(klass, "GetFloatVals");
            if (valsMethod && !keys.empty())
            {
                MonoArray* valsArr = (MonoArray*)mono_runtime_invoke(valsMethod, m_ManagedInstance, nullptr, nullptr);
                if (valsArr)
                    for (int i = 0; i < (int)keys.size(); i++)
                    {
                        float val = mono_array_get(valsArr, float, i);
                        CreateFloatValue(keys[i], val);
                    }
            }
        }
        // String
        {
            auto keys = getStringArray("GetStringKeys");
            auto vals = getStringArray("GetStringVals");
            for (int i = 0; i < (int)keys.size() && i < (int)vals.size(); i++)
                CreateStringValue(keys[i], vals[i]);
        }

        LOG_CORE_INFO("ManagedBTBlackboard synced: {} bools, {} ints, {} floats, {} strings", GetBoolValues().size(), GetIntValues().size(), GetFloatValues().size(), GetStringValues().size());
    }

    //---------------------BTAction---------------------
    
    ManagedBTAction::ManagedBTAction(const std::string& name, const std::string& managedClassName, MonoObject* paramsInstance)
        : HActionNode(name), m_ManagedClassName(managedClassName), m_ParamsInstance(paramsInstance)
    {
        m_ManagedInstance = ScriptEngine::CreateBTActionInstance(managedClassName);
        
        if (m_ManagedInstance && m_ParamsInstance)
        {
            MonoClass* klass = mono_object_get_class(m_ManagedInstance);
            MonoMethod* setParamsMethod = mono_class_get_method_from_name(klass, "SetParameters", 1);
            if (setParamsMethod)
            {
                void* args[1] = { m_ParamsInstance };
                mono_runtime_invoke(setParamsMethod, m_ManagedInstance, args, nullptr);
            }
        }
    }

    ManagedBTAction::~ManagedBTAction()
    {
    }

    void ManagedBTAction::OnStart()
    {
        HActionNode::OnStart();
        
        if (m_ManagedInstance)
        {
            InitializeManagedNode();
            ScriptEngine::CallBTNodeOnStart(m_ManagedInstance);
        }
    }

    NodeStatus ManagedBTAction::Update()
    {
        if (!CheckConditionsSelfMode())
            return NodeStatus::FAILURE;

        if (m_ManagedInstance)
        {
            int result = ScriptEngine::CallBTNodeUpdate(m_ManagedInstance);
            return (NodeStatus)result;
        }
        return NodeStatus::FAILURE;
    }

    void ManagedBTAction::OnFinished()
    {
        if (m_ManagedInstance)
            ScriptEngine::CallBTNodeOnFinished(m_ManagedInstance);
        HActionNode::OnFinished();
    }

    void ManagedBTAction::OnAbort()
    {
        if (m_ManagedInstance)
            ScriptEngine::CallBTNodeOnAbort(m_ManagedInstance);
        HActionNode::OnAbort();
    }

    void ManagedBTAction::InitializeManagedNode()
    {
        if (!GetTree())
            return;

        auto* tree = GetTree();
        /*Entity* owner = tree->GetOwner<Entity>();
        
        if (!owner)
            return;

        UUID entityID = owner->GetUUID();*/
        UUID* ownerUUID = tree->GetOwner<UUID>();
        if (!ownerUUID)
            return;
        
        MonoObject* managedBlackboard = nullptr;
        auto* rawBlackboard = tree->GetBlackboardRaw();
        
        if (auto* managedBB = dynamic_cast<ManagedBTBlackboard*>(rawBlackboard))
        {
            managedBlackboard = managedBB->GetManagedInstance();
        }

        ScriptEngine::InitializeBTNode(m_ManagedInstance, managedBlackboard, *ownerUUID);
    }

    //---------------------BTCondition---------------------
    
    ManagedBTCondition::ManagedBTCondition(const std::string& name, const std::string& managedClassName, MonoObject* paramsInstance)
        : HCondition(name), m_ManagedClassName(managedClassName), m_ParamsInstance(paramsInstance)
    {
        m_ManagedInstance = ScriptEngine::CreateBTConditionInstance(managedClassName);
        
        if (m_ManagedInstance && m_ParamsInstance)
        {
            MonoClass* klass = mono_object_get_class(m_ManagedInstance);
            MonoMethod* setParamsMethod = mono_class_get_method_from_name(klass, "SetParameters", 1);
            if (setParamsMethod)
            {
                void* args[1] = { m_ParamsInstance };
                mono_runtime_invoke(setParamsMethod, m_ManagedInstance, args, nullptr);
            }
        }
    }

    void ManagedBTCondition::OnStart()
    {
        if (m_ManagedInstance)
        {
            InitializeManagedNode();
            ScriptEngine::CallBTNodeOnStart(m_ManagedInstance);
        }
    }

    bool ManagedBTCondition::CheckCondition()
    {
        if (m_ManagedInstance)
            return ScriptEngine::CallBTConditionCheck(m_ManagedInstance);
        return false;
    }

    void ManagedBTCondition::OnFinished()
    {
        if (m_ManagedInstance)
            ScriptEngine::CallBTNodeOnFinished(m_ManagedInstance);
    }

    void ManagedBTCondition::OnAbort()
    {
        if (m_ManagedInstance)
            ScriptEngine::CallBTNodeOnAbort(m_ManagedInstance);
    }

    void ManagedBTCondition::InitializeManagedNode()
    {
        if (!GetTree())
            return;

        auto* tree = GetTree();
        /*Entity* owner = tree->GetOwner<Entity>();
        
        if (!owner)
            return;

        UUID entityID = owner->GetUUID();*/
        UUID* ownerUUID = tree->GetOwner<UUID>();
        if (!ownerUUID)
            return;
        
        MonoObject* managedBlackboard = nullptr;
        auto* rawBlackboard = tree->GetBlackboardRaw();
        
        if (auto* managedBB = dynamic_cast<ManagedBTBlackboard*>(rawBlackboard))
        {
            managedBlackboard = managedBB->GetManagedInstance();
        }

        ScriptEngine::InitializeBTNode(m_ManagedInstance, managedBlackboard, *ownerUUID);
    }

    //---------------------BTDecorator---------------------
    
    ManagedBTDecorator::ManagedBTDecorator(const std::string& name, const std::string& managedClassName, MonoObject* paramsInstance)
        : HDecorator(name), m_ManagedClassName(managedClassName), m_ParamsInstance(paramsInstance)
    {
        m_ManagedInstance = ScriptEngine::CreateBTDecoratorInstance(managedClassName);
        
        if (m_ManagedInstance && m_ParamsInstance)
        {
            MonoClass* klass = mono_object_get_class(m_ManagedInstance);
            MonoMethod* setParamsMethod = mono_class_get_method_from_name(klass, "SetParameters", 1);
            if (setParamsMethod)
            {
                void* args[1] = { m_ParamsInstance };
                mono_runtime_invoke(setParamsMethod, m_ManagedInstance, args, nullptr);
            }
        }
    }

    void ManagedBTDecorator::OnStart()
    {
        if (m_ManagedInstance)
        {
            InitializeManagedNode();
            ScriptEngine::CallBTNodeOnStart(m_ManagedInstance);
        }
    }

    bool ManagedBTDecorator::CanExecute()
    {
        if (m_ManagedInstance)
            return ScriptEngine::CallBTDecoratorCanExecute(m_ManagedInstance);
        return true;
    }

    void ManagedBTDecorator::OnFinishedResult(NodeStatus& status)
    {
        if (m_ManagedInstance)
            ScriptEngine::CallBTDecoratorOnFinishedResult(m_ManagedInstance, status);
    }

    void ManagedBTDecorator::OnFinished()
    {
        if (m_ManagedInstance)
            ScriptEngine::CallBTNodeOnFinished(m_ManagedInstance);
    }

    void ManagedBTDecorator::OnAbort()
    {
        if (m_ManagedInstance)
            ScriptEngine::CallBTNodeOnAbort(m_ManagedInstance);
    }

    void ManagedBTDecorator::InitializeManagedNode()
    {
        if (!GetTree())
            return;

        auto* tree = GetTree();
        /*Entity* owner = tree->GetOwner<Entity>();
        
        if (!owner)
            return;

        UUID entityID = owner->GetUUID();*/
        UUID* ownerUUID = tree->GetOwner<UUID>();
        if (!ownerUUID)
            return;
        
        MonoObject* managedBlackboard = nullptr;
        auto* rawBlackboard = tree->GetBlackboardRaw();
        
        if (auto* managedBB = dynamic_cast<ManagedBTBlackboard*>(rawBlackboard))
        {
            managedBlackboard = managedBB->GetManagedInstance();
        }

        ScriptEngine::InitializeBTNode(m_ManagedInstance, managedBlackboard, *ownerUUID);
    }
}