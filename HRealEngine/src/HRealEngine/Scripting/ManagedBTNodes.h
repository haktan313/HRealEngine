#pragma once
#include <mono/metadata/object.h>
#include "BehaviorTreeThings/Core/Tree.h"


namespace HRealEngine
{
    class Entity;
    class ScriptEngine;
    
    class ManagedBTBlackboard : public HBlackboard
    {
    public:
        ManagedBTBlackboard(const std::string& name, const std::string& managedClassName);
        ~ManagedBTBlackboard();
        void SyncFromManagedBlackboard();
        void SyncToManagedBlackboard();
        MonoObject* GetManagedInstance() const { return m_ManagedInstance; }

    private:
        std::string m_ManagedClassName;
        MonoObject* m_ManagedInstance;
    };

    class ManagedBTAction : public HActionNode
    {
    public:
        ManagedBTAction(const std::string& name, const std::string& managedClassName, MonoObject* paramsInstance = nullptr);
        ~ManagedBTAction();

        void OnStart() override;
        NodeStatus Update() override;
        void OnFinished() override;
        void OnAbort() override;

        MonoObject* GetManagedInstance() const { return m_ManagedInstance; }
        MonoObject* GetParametersInstance() const { return m_ParamsInstance; }

        void SetParametersInstance(void* p);
    private:
        void InitializeManagedNode();

        std::string m_ManagedClassName;
        MonoObject* m_ManagedInstance;
        MonoObject* m_ParamsInstance;
    };

    class ManagedBTCondition : public HCondition
    {
    public:
        ManagedBTCondition(const std::string& name, const std::string& managedClassName, MonoObject* paramsInstance = nullptr);

        void OnStart() override;
        bool CheckCondition() override;
        void OnFinished() override;
        void OnAbort() override;

        MonoObject* GetManagedInstance() const { return m_ManagedInstance; }
        MonoObject* GetParametersInstance() const { return m_ParamsInstance; }

        void SetParametersInstance(void* p);
    private:
        void InitializeManagedNode();

        std::string m_ManagedClassName;
        MonoObject* m_ManagedInstance;
        MonoObject* m_ParamsInstance;
    };

    class ManagedBTDecorator : public HDecorator
    {
    public:
        ManagedBTDecorator(const std::string& name, const std::string& managedClassName, MonoObject* paramsInstance = nullptr);

        void OnStart() override;
        bool CanExecute() override;
        void OnFinishedResult(NodeStatus& status) override;
        void OnFinished() override;
        void OnAbort() override;

        MonoObject* GetManagedInstance() const { return m_ManagedInstance; }
        MonoObject* GetParametersInstance() const { return m_ParamsInstance; }

        void SetParametersInstance(void* p);
    private:
        void InitializeManagedNode();

        std::string m_ManagedClassName;
        MonoObject* m_ManagedInstance;
        MonoObject* m_ParamsInstance;
    };
}