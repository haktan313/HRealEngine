#include "HRpch.h"
#include "ScriptBehaviorTreeNodes.h"

#include "ScriptEngine.h"
#include "BehaviorTreeThings/Core/NodeRegistry.h"
#include "HRealEngine/Core/Entity.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/mono-gc.h>

namespace HRealEngine
{
    namespace
    {

        inline thread_local uint64_t s_CurrentBehaviorTreeOwnerID = 0;

        class ScopedBehaviorTreeOwner
        {
        public:
            explicit ScopedBehaviorTreeOwner(const HNode* node)
            {
                m_PreviousOwner = s_CurrentBehaviorTreeOwnerID;
                if (!node)
                {
                    s_CurrentBehaviorTreeOwnerID = 0;
                    return;
                }

                Entity* owner = node->GetOwner<Entity>();
                if (owner && *owner)
                    s_CurrentBehaviorTreeOwnerID = owner->GetUUID();
                else
                    s_CurrentBehaviorTreeOwnerID = 0;
            }

            ~ScopedBehaviorTreeOwner()
            {
                s_CurrentBehaviorTreeOwnerID = m_PreviousOwner;
            }

        private:
            uint64_t m_PreviousOwner = 0;
        };
        void SplitManagedTypeName(const std::string& managedTypeName, std::string& outNamespace, std::string& outClassName)
        {
            size_t separator = managedTypeName.find_last_of('.');
            if (separator == std::string::npos)
            {
                outNamespace.clear();
                outClassName = managedTypeName;
                return;
            }

            outNamespace = managedTypeName.substr(0, separator);
            outClassName = managedTypeName.substr(separator + 1);
        }

        MonoClass* GetManagedClass(const std::string& managedTypeName)
        {
            std::string classNamespace;
            std::string className;
            SplitManagedTypeName(managedTypeName, classNamespace, className);

            MonoImage* appImage = ScriptEngine::GetAppAssemblyImage();
            if (!appImage)
                return nullptr;

            return mono_class_from_name(appImage, classNamespace.c_str(), className.c_str());
        }

        class ScriptBTManagedInstance
        {
        public:
            explicit ScriptBTManagedInstance(const std::string& managedTypeName)
                : m_ManagedTypeName(managedTypeName)
            {
                MonoClass* klass = GetManagedClass(managedTypeName);
                if (!klass)
                {
                    LOG_CORE_ERROR("Could not find managed BT class '{}'.", managedTypeName);
                    return;
                }

                MonoObject* instance = mono_object_new(mono_domain_get(), klass);
                if (instance)
                    mono_runtime_object_init(instance);
                if (!instance)
                    return;

                m_GCHandle = mono_gchandle_new(instance, false);
                m_Class = klass;
            }

            ~ScriptBTManagedInstance()
            {
                if (m_GCHandle != 0)
                    mono_gchandle_free(m_GCHandle);
            }

            MonoObject* GetObject() const
            {
                if (m_GCHandle == 0)
                    return nullptr;
                return mono_gchandle_get_target(m_GCHandle);
            }

            MonoMethod* GetMethod(const std::string& methodName, int paramCount)
            {
                if (!m_Class)
                    return nullptr;
                return mono_class_get_method_from_name(m_Class, methodName.c_str(), paramCount);
            }

        private:
            std::string m_ManagedTypeName;
            uint32_t m_GCHandle = 0;
            MonoClass* m_Class = nullptr;
        };

        class ScriptActionParameters : public ParamsForAction
        {
        public:
            explicit ScriptActionParameters(const std::string& managedTypeName = "")
                : ManagedTypeName(managedTypeName)
            {
            }

            void DrawImGui(HBlackboard* blackboard) override
            {
                (void)blackboard;
                DrawStringValue("Managed Type", ManagedTypeName);
            }

            void Serialize(YAML::Emitter& out) const override
            {
                SerializeString("ManagedTypeName", ManagedTypeName, out);
            }

            void Deserialize(const YAML::Node& node) override
            {
                DeserializeString(node, "ManagedTypeName", ManagedTypeName);
            }

            std::string ManagedTypeName;
        };

        class ScriptConditionParameters : public ParamsForCondition
        {
        public:
            explicit ScriptConditionParameters(const std::string& managedTypeName = "")
                : ManagedTypeName(managedTypeName)
            {
            }

            void DrawImGui(HBlackboard* blackboard) override
            {
                (void)blackboard;
                DrawStringValue("Managed Type", ManagedTypeName);
            }

            void Serialize(YAML::Emitter& out) const override
            {
                SerializeString("ManagedTypeName", ManagedTypeName, out);
            }

            void Deserialize(const YAML::Node& node) override
            {
                DeserializeString(node, "ManagedTypeName", ManagedTypeName);
            }

            std::string ManagedTypeName;
        };

        class ScriptDecoratorParameters : public ParamsForDecorator
        {
        public:
            explicit ScriptDecoratorParameters(const std::string& managedTypeName = "")
                : ManagedTypeName(managedTypeName)
            {
            }

            void DrawImGui(HBlackboard* blackboard) override
            {
                (void)blackboard;
                DrawStringValue("Managed Type", ManagedTypeName);
            }

            void Serialize(YAML::Emitter& out) const override
            {
                SerializeString("ManagedTypeName", ManagedTypeName, out);
            }

            void Deserialize(const YAML::Node& node) override
            {
                DeserializeString(node, "ManagedTypeName", ManagedTypeName);
            }

            std::string ManagedTypeName;
        };

        class ScriptActionNode : public HActionNode
        {
        public:
            ScriptActionNode(const std::string& name, const ScriptActionParameters& parameters = ScriptActionParameters())
                : HActionNode(name, parameters), m_Instance(parameters.ManagedTypeName)
            {
                SetParams<ScriptActionParameters>(parameters);
                m_OnStart = m_Instance.GetMethod("OnStart", 0);
                m_OnUpdate = m_Instance.GetMethod("OnUpdate", 0);
                m_OnFinished = m_Instance.GetMethod("OnFinished", 0);
                m_OnAbort = m_Instance.GetMethod("OnAbort", 0);
            }

            void OnStart() override
            {
                InvokeVoid(m_OnStart);
            }

            NodeStatus Update() override
            {
                int value = InvokeInt(m_OnUpdate, static_cast<int>(NodeStatus::FAILURE));
                return static_cast<NodeStatus>(value);
            }

            void OnFinished() override
            {
                InvokeVoid(m_OnFinished);
            }

            void OnAbort() override
            {
                InvokeVoid(m_OnAbort);
            }

        private:
            void InvokeVoid(MonoMethod* method)
            {
                if (!method)
                    return;
                ScopedBehaviorTreeOwner ownerScope(this);
                MonoObject* instance = m_Instance.GetObject();
                if (!instance)
                    return;
                mono_runtime_invoke(method, instance, nullptr, nullptr);
            }

            int InvokeInt(MonoMethod* method, int fallbackValue)
            {
                if (!method)
                    return fallbackValue;
                ScopedBehaviorTreeOwner ownerScope(this);
                MonoObject* instance = m_Instance.GetObject();
                if (!instance)
                    return fallbackValue;

                MonoObject* result = mono_runtime_invoke(method, instance, nullptr, nullptr);
                if (!result)
                    return fallbackValue;

                return *(int*)mono_object_unbox(result);
            }

            ScriptBTManagedInstance m_Instance;
            MonoMethod* m_OnStart = nullptr;
            MonoMethod* m_OnUpdate = nullptr;
            MonoMethod* m_OnFinished = nullptr;
            MonoMethod* m_OnAbort = nullptr;
        };

        class ScriptConditionNode : public HCondition
        {
        public:
            ScriptConditionNode(const std::string& name, const ScriptConditionParameters& parameters = ScriptConditionParameters())
                : HCondition(name, parameters), m_Instance(parameters.ManagedTypeName)
            {
                SetParams<ScriptConditionParameters>(parameters);
                m_CheckCondition = m_Instance.GetMethod("CheckCondition", 0);
            }

            bool CheckCondition() override
            {
                if (!m_CheckCondition)
                    return false;
                ScopedBehaviorTreeOwner ownerScope(this);
                MonoObject* instance = m_Instance.GetObject();
                if (!instance)
                    return false;

                MonoObject* result = mono_runtime_invoke(m_CheckCondition, instance, nullptr, nullptr);
                if (!result)
                    return false;

                return *(bool*)mono_object_unbox(result);
            }

        private:
            ScriptBTManagedInstance m_Instance;
            MonoMethod* m_CheckCondition = nullptr;
        };

        class ScriptDecoratorNode : public HDecorator
        {
        public:
            ScriptDecoratorNode(const std::string& name, const ScriptDecoratorParameters& parameters = ScriptDecoratorParameters())
                : HDecorator(name, parameters), m_Instance(parameters.ManagedTypeName)
            {
                SetParams<ScriptDecoratorParameters>(parameters);
                m_CanExecute = m_Instance.GetMethod("CanExecute", 0);
                m_OnFinishedResult = m_Instance.GetMethod("OnFinishedResult", 1);
            }

            bool CanExecute() override
            {
                if (!m_CanExecute)
                    return true;
                ScopedBehaviorTreeOwner ownerScope(this);
                MonoObject* instance = m_Instance.GetObject();
                if (!instance)
                    return true;

                MonoObject* result = mono_runtime_invoke(m_CanExecute, instance, nullptr, nullptr);
                if (!result)
                    return true;

                return *(bool*)mono_object_unbox(result);
            }

            void OnFinishedResult(NodeStatus& status) override
            {
                if (!m_OnFinishedResult)
                    return;
                ScopedBehaviorTreeOwner ownerScope(this);
                MonoObject* instance = m_Instance.GetObject();
                if (!instance)
                    return;

                int statusAsInt = static_cast<int>(status);
                void* args[1] = { &statusAsInt };
                MonoObject* result = mono_runtime_invoke(m_OnFinishedResult, instance, args, nullptr);
                if (!result)
                    return;

                int newStatus = *(int*)mono_object_unbox(result);
                status = static_cast<NodeStatus>(newStatus);
            }

        private:
            ScriptBTManagedInstance m_Instance;
            MonoMethod* m_CanExecute = nullptr;
            MonoMethod* m_OnFinishedResult = nullptr;
        };

        class ScriptBlackboard : public HBlackboard
        {
        public:
            ScriptBlackboard(const std::string& blackboardName, const std::string& managedTypeName)
                : HBlackboard(blackboardName), m_Instance(managedTypeName)
            {
                m_OnCreate = m_Instance.GetMethod("OnCreate", 0);
                s_CurrentBlackboard = this;
                InvokeVoid(m_OnCreate);
                s_CurrentBlackboard = nullptr;
            }

            void AddBoolValue(const std::string& key, bool value) { CreateBoolValue(key, value); }
            void AddIntValue(const std::string& key, int value) { CreateIntValue(key, value); }
            void AddFloatValue(const std::string& key, float value) { CreateFloatValue(key, value); }
            void AddStringValue(const std::string& key, const std::string& value) { CreateStringValue(key, value); }

            static ScriptBlackboard* GetCurrentBlackboard() { return s_CurrentBlackboard; }

        private:
            void InvokeVoid(MonoMethod* method)
            {
                if (!method)
                    return;
                ScopedBehaviorTreeOwner ownerScope(nullptr);
                MonoObject* instance = m_Instance.GetObject();
                if (!instance)
                    return;
                mono_runtime_invoke(method, instance, nullptr, nullptr);
            }

            ScriptBTManagedInstance m_Instance;
            MonoMethod* m_OnCreate = nullptr;
            inline static thread_local ScriptBlackboard* s_CurrentBlackboard = nullptr;
        };
    }

    void RegisterScriptBehaviorTreeAction(const std::string& displayName, const std::string& managedTypeName)
    {
        ActionClassInfo info;
        info.Name = displayName;
        info.CreateParamsFn = [managedTypeName]()
        {
            return std::make_unique<ScriptActionParameters>(managedTypeName);
        };
        info.BuildFn = [](BehaviorTreeBuilder& builder, Node* node, ParamsForAction& baseParams)
        {
            auto& params = static_cast<ScriptActionParameters&>(baseParams);
            builder.action<ScriptActionNode>(node->Name, params);
        };
        info.BuildFromYAML = [managedTypeName](BehaviorTreeBuilder& builder, const std::string& instanceName, const YAML::Node& paramsNode)
        {
            ScriptActionParameters params(managedTypeName);
            params.Deserialize(paramsNode);
            if (params.ManagedTypeName.empty())
                params.ManagedTypeName = managedTypeName;
            builder.action<ScriptActionNode>(instanceName, params);
        };

        NodeRegistry::AddActionNodeInfo(displayName, std::move(info));
    }

    void RegisterScriptBehaviorTreeCondition(const std::string& displayName, const std::string& managedTypeName)
    {
        ConditionClassInfo info;
        info.Name = displayName;
        info.CreateParamsFn = [managedTypeName]()
        {
            return std::make_unique<ScriptConditionParameters>(managedTypeName);
        };
        info.BuildFn = [displayName](BehaviorTreeBuilder& builder, ParamsForCondition& baseParams)
        {
            auto& params = static_cast<ScriptConditionParameters&>(baseParams);
            builder.condition<ScriptConditionNode>(baseParams.Priority, displayName, params);
        };
        info.BuildFromYAML = [managedTypeName](BehaviorTreeBuilder& builder, const std::string& instanceName, const YAML::Node& paramsNode, PriorityType priority)
        {
            ScriptConditionParameters params(managedTypeName);
            params.Deserialize(paramsNode);
            if (params.ManagedTypeName.empty())
                params.ManagedTypeName = managedTypeName;
            builder.condition<ScriptConditionNode>(priority, instanceName, params);
        };

        NodeRegistry::AddConditionNodeInfo(displayName, std::move(info));
    }

    void RegisterScriptBehaviorTreeDecorator(const std::string& displayName, const std::string& managedTypeName)
    {
        DecoratorClassInfo info;
        info.Name = displayName;
        info.CreateParamsFn = [managedTypeName]()
        {
            return std::make_unique<ScriptDecoratorParameters>(managedTypeName);
        };
        info.BuildFn = [displayName](BehaviorTreeBuilder& builder, ParamsForDecorator& baseParams)
        {
            auto& params = static_cast<ScriptDecoratorParameters&>(baseParams);
            builder.decorator<ScriptDecoratorNode>(displayName, params);
        };
        info.BuildFromYAML = [managedTypeName](BehaviorTreeBuilder& builder, const std::string& instanceName, const YAML::Node& paramsNode)
        {
            ScriptDecoratorParameters params(managedTypeName);
            params.Deserialize(paramsNode);
            if (params.ManagedTypeName.empty())
                params.ManagedTypeName = managedTypeName;
            builder.decorator<ScriptDecoratorNode>(instanceName, params);
        };

        NodeRegistry::AddDecoratorNodeInfo(displayName, std::move(info));
    }

    void RegisterScriptBehaviorTreeBlackboard(const std::string& displayName, const std::string& managedTypeName)
    {
        BlackboardClassInfo info;
        info.Name = displayName;
        info.CreateBlackboardFn = [displayName, managedTypeName]()
        {
            return std::make_unique<ScriptBlackboard>(displayName, managedTypeName);
        };

        NodeRegistry::AddBlackboardInfo(displayName, std::move(info));
    }

    bool CreateScriptBlackboardBool(const std::string& key, bool value)
    {
        ScriptBlackboard* bb = ScriptBlackboard::GetCurrentBlackboard();
        if (!bb)
            return false;
        bb->AddBoolValue(key, value);
        return true;
    }

    bool CreateScriptBlackboardInt(const std::string& key, int value)
    {
        ScriptBlackboard* bb = ScriptBlackboard::GetCurrentBlackboard();
        if (!bb)
            return false;
        bb->AddIntValue(key, value);
        return true;
    }

    bool CreateScriptBlackboardFloat(const std::string& key, float value)
    {
        ScriptBlackboard* bb = ScriptBlackboard::GetCurrentBlackboard();
        if (!bb)
            return false;
        bb->AddFloatValue(key, value);
        return true;
    }

    bool CreateScriptBlackboardString(const std::string& key, const std::string& value)
    {
        ScriptBlackboard* bb = ScriptBlackboard::GetCurrentBlackboard();
        if (!bb)
            return false;
        bb->AddStringValue(key, value);
        return true;
    }

    uint64_t GetCurrentBehaviorTreeOwnerEntityID()
    {
        return s_CurrentBehaviorTreeOwnerID;
    }
}
