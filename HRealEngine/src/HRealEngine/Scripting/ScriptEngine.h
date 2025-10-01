#pragma once
#include <filesystem>
#include <mono/metadata/object-forward.h>

#include "HRealEngine/Scene/Scene.h"

extern "C"
{
    typedef struct _MonoClass MonoClass;
    typedef struct _MonoObject MonoObject;
    typedef struct _MonoMethod MonoMethod;
    typedef struct _MonoAssembly MonoAssembly;
}

namespace HRealEngine
{

    class ScriptClass
    {
    public:
        ScriptClass() = default;
        ScriptClass(const std::string& classNamespace, const std::string& className);

        MonoObject* Instantiate();
        MonoMethod* GetMethod(const std::string& methodName, int paramCount);
        MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);
    private:
        std::string m_ClassNamespace;
        std::string m_ClassName;
        MonoClass* m_MonoClass = nullptr;
    };

    class ScriptInstance
    {
    public:
        ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);

        void InvokeOnCreate();
        void InvokeOnUpdate(Timestep ts);
    private:
        Ref<ScriptClass> m_ScriptClass;
        MonoObject* m_Instance = nullptr;
        MonoMethod* m_Constructor = nullptr;
        MonoMethod* m_OnCreateMethod = nullptr;
        MonoMethod* m_OnUpdateMethod = nullptr;
    };
    
    class ScriptEngine
    {
    public:
        static void Init();
        static void Shutdown();

        static void LoadAssembly(const std::filesystem::path& assemblyPath);
        static void LoadAssemblyClasses(MonoAssembly* assembly);

        static void OnRuntimeStart(Scene* scene);
        static void OnRuntimeStop();

        static bool IsEntityClassExist(const std::string& className);
        static void OnCreateEntity(Entity entity);
        static void OnUpdateEntity(Entity entity, Timestep ts);

        static Scene* GetSceneContext();
        static std::unordered_map<std::string, Ref<ScriptClass>> GetEntityClasses();
        static MonoImage* GetCoreAssemblyImage();
    private:
        static void InitMono();
        static void ShutdownMono();

        static MonoObject* InstantiateClass(MonoClass* monoClass);
        friend class ScriptClass;
    };
}
