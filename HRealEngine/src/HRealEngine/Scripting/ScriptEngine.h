#pragma once
#include <filesystem>
#include <mono/metadata/class.h>
#include <mono/metadata/object-forward.h>

#include "HRealEngine/Scene/Scene.h"

extern "C"
{
    typedef struct _MonoClass MonoClass;
    typedef struct _MonoObject MonoObject;
    typedef struct _MonoMethod MonoMethod;
    typedef struct _MonoAssembly MonoAssembly;
    typedef struct _MonoImage MonoImage;
    typedef struct _MonoClassField MonoClassField;
}

namespace HRealEngine
{
    enum class ScriptFieldType
    {
        None = 0, Float, Double, Bool, Char, Byte,
        Short, Int, Long, UByte, UShort, UInt, ULong,
        Vector2, Vector3, Vector4,
        Entity
    };
    struct ScriptField
    {
        ScriptFieldType Type;
        std::string Name;
        MonoClassField* ClassField;
    };

    
    class ScriptClass
    {
    public:
        ScriptClass() = default;
        ScriptClass(const std::string& classNamespace, const std::string& className, bool bIsCore = false);

        MonoObject* Instantiate();
        MonoMethod* GetMethod(const std::string& methodName, int paramCount);
        MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);

        const std::map<std::string, ScriptField>& GetFields() const { return m_Fields; }
    private:
        std::string m_ClassNamespace;
        std::string m_ClassName;
        std::map<std::string, ScriptField> m_Fields;
        MonoClass* m_MonoClass = nullptr;
        friend class ScriptEngine;
    };

    class ScriptInstance
    {
    public:
        ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);

        void InvokeOnCreate();
        void InvokeOnUpdate(Timestep ts);

        Ref<ScriptClass> GetScriptClass() const { return m_ScriptClass; }

        template<typename T>
        T GetFieldValue(const std::string& name)
        {
            T value;
            bool success = GetFieldValueInternal(name, &value);
            if (!success)
                return T();
            return *(T*)s_FieldValueBuffer;
        }
        template<typename T>
        void SetFieldValue(const std::string& name, const T& value)
        {
            SetFieldValueInternal(name, &value);
        }
    private:
        bool GetFieldValueInternal(const std::string& name, void* outValue);
        bool SetFieldValueInternal(const std::string& name, const void* value);
        
        Ref<ScriptClass> m_ScriptClass;
        MonoObject* m_Instance = nullptr;
        MonoMethod* m_Constructor = nullptr;
        MonoMethod* m_OnCreateMethod = nullptr;
        MonoMethod* m_OnUpdateMethod = nullptr;

            inline static char s_FieldValueBuffer[8];
    };
    
    class ScriptEngine
    {
    public:
        static void Init();
        static void Shutdown();

        static void LoadAssembly(const std::filesystem::path& assemblyPath);
        static void LoadAppAssembly(const std::filesystem::path& assemblyPath);
        static void LoadAssemblyClasses(/*MonoAssembly* assembly*/);

        static void OnRuntimeStart(Scene* scene);
        static void OnRuntimeStop();

        static bool IsEntityClassExist(const std::string& className);
        static void OnCreateEntity(Entity entity);
        static void OnUpdateEntity(Entity entity, Timestep ts);

        static Scene* GetSceneContext();
        static std::unordered_map<std::string, Ref<ScriptClass>> GetEntityClasses();
        static MonoImage* GetCoreAssemblyImage();
        static Ref<ScriptInstance> GetEntitySriptInstance(UUID entityID);
    private:
        static void InitMono();
        static void ShutdownMono();

        static MonoObject* InstantiateClass(MonoClass* monoClass);
        friend class ScriptClass;
        friend class ScriptGlue;
    };
}
