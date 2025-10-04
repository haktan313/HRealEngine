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
    struct ScriptFieldInstance
    {
        ScriptField Field;
        ScriptFieldInstance()
        {
            memset(m_Buffer, 0, sizeof(m_Buffer));
        }

        template<typename T>
        T GetValue()
        {
            //static_assert(sizeof(T) <= sizeof(m_Buffer), "ScriptFieldInstance::GetValue() buffer overflow");
            return *(T*)m_Buffer;
        }
        template<typename T>
        void SetValue(const T& value)
        {
            //static_assert(sizeof(T) <= sizeof(m_Buffer), "ScriptFieldInstance::SetValue() buffer overflow");
            memcpy(m_Buffer, &value, sizeof(T));
        }
    private:
        uint8_t m_Buffer[8];
        friend class ScriptEngine;
        friend class ScriptInstance;
    };

    using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;
    
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
        void SetFieldValue(const std::string& name, T value)
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

        inline static char s_FieldValueBuffer[16];

        friend class ScriptEngine;
        friend struct ScriptFieldInstance;
    };
    
    class ScriptEngine 
    {
    public:
        static const char* ScriptFieldTypeToString(ScriptFieldType type)
        {
            switch (type)
            {
            case ScriptFieldType::None:       return "None";
            case ScriptFieldType::Float:      return "Float";
            case ScriptFieldType::Double:     return "Double";
            case ScriptFieldType::Bool:       return "Bool";
            case ScriptFieldType::Char:       return "Char";
            case ScriptFieldType::Byte:       return "Byte";
            case ScriptFieldType::Short:      return "Short";
            case ScriptFieldType::Int:        return "Int";
            case ScriptFieldType::Long:       return "Long";
            case ScriptFieldType::UByte:      return "UByte";
            case ScriptFieldType::UShort:     return "UShort";
            case ScriptFieldType::UInt:       return "UInt";
            case ScriptFieldType::ULong:      return "ULong";
            case ScriptFieldType::Vector2:    return "Vector2";
            case ScriptFieldType::Vector3:    return "Vector3";
            case ScriptFieldType::Vector4:    return "Vector4";
            case ScriptFieldType::Entity:     return "Entity";
            }
            HREALENGINE_CORE_DEBUGBREAK(false, "Unknown script field type");
            return "<Invalid>";
        }
    
        static inline ScriptFieldType ScriptFieldTypeFromString(const std::string& type)
        {
            if (type == "None")       return ScriptFieldType::None;
            if (type == "Float")      return ScriptFieldType::Float;
            if (type == "Double")     return ScriptFieldType::Double;
            if (type == "Bool")       return ScriptFieldType::Bool;
            if (type == "Char")       return ScriptFieldType::Char;
            if (type == "Byte")       return ScriptFieldType::Byte;
            if (type == "Short")      return ScriptFieldType::Short;
            if (type == "Int")        return ScriptFieldType::Int;
            if (type == "Long")       return ScriptFieldType::Long;
            if (type == "UByte")      return ScriptFieldType::UByte;
            if (type == "UShort")     return ScriptFieldType::UShort;
            if (type == "UInt")       return ScriptFieldType::UInt;
            if (type == "ULong")      return ScriptFieldType::ULong;
            if (type == "Vector2")    return ScriptFieldType::Vector2;
            if (type == "Vector3")    return ScriptFieldType::Vector3;
            if (type == "Vector4")    return ScriptFieldType::Vector4;
            if (type == "Entity")     return ScriptFieldType::Entity;
            HREALENGINE_CORE_DEBUGBREAK(false, "Unknown script field type");
            return ScriptFieldType::None;
        }
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
        static Ref<ScriptClass> GetEntityClass(const std::string& className);
        static ScriptFieldMap& GetScriptFieldMap(Entity entity);
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
