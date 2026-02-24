#pragma once
#include <filesystem>
#include <mono/metadata/class.h>
#include <mono/metadata/object-forward.h>

#include "ManagedBTNodes.h"
#include "HRealEngine/Scene/Scene.h"

namespace JPH
{
    class BodyInterface;
}

extern "C"
{
    typedef struct _MonoClass MonoClass;
    typedef struct _MonoObject MonoObject;
    typedef struct _MonoMethod MonoMethod;
    typedef struct _MonoAssembly MonoAssembly;
    typedef struct _MonoImage MonoImage;
    typedef struct _MonoClassField MonoClassField;
    typedef struct _MonoString MonoString;
}

namespace HRealEngine
{
    enum class ScriptFieldType
    {
        None = 0, Float, Double, Bool, Char, Byte,
        Short, Int, Long, UByte, UShort, UInt, ULong,
        Vector2, Vector3, Vector4,
        Entity, String
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

        template<> std::string GetValue<std::string>() { return m_StringStorage; }
        template<> void SetValue<std::string>(const std::string& v) { m_StringStorage = v; }
    private:
        uint8_t m_Buffer[8];
        std::string m_StringStorage;
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

        void InvokeBeginPlay();
        void InvokeOnDestroy();
        void InvokeTick(Timestep ts);
        void InvokeOnCollisionEnter(UUID otherID);

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

        MonoObject* GetManagedObject() const { return m_Instance; }
    private:
        void InvokeOnCollisionExit(UUID otherID);
        bool GetFieldValueInternal(const std::string& name, void* outValue);
        bool SetFieldValueInternal(const std::string& name, const void* value);
        
        Ref<ScriptClass> m_ScriptClass;
        MonoObject* m_Instance = nullptr;
        MonoMethod* m_Constructor = nullptr;
        MonoMethod* m_BeginPlayMethod = nullptr;
        MonoMethod* m_TickMethod = nullptr;
        
        MonoMethod* m_OnCollisionEnterMethod = nullptr;
        MonoMethod* m_OnCollisionExitMethod = nullptr;
        MonoMethod* m_OnDestroyMethod = nullptr;

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
            case ScriptFieldType::String:     return "String";
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
            if (type == "String")     return ScriptFieldType::String;
            HREALENGINE_CORE_DEBUGBREAK(false, "Unknown script field type");
            return ScriptFieldType::None;
        }
        static void Init();
        static void Shutdown();

        static bool IsInitialized();
        static bool LoadAssembly(const std::filesystem::path& assemblyPath);
        static bool LoadAppAssembly(const std::filesystem::path& assemblyPath);
        static void LoadAssemblyClasses(/*MonoAssembly* assembly*/);

        static void InitCSharpProject();
        static void ReloadAssembly();

        static void OnRuntimeStart(Scene* scene);
        static void OnRuntimeStop();
        static void SetBodyInterface(JPH::BodyInterface* bodyInterface);

        static bool IsEntityClassExist(const std::string& className);
        static void OnCreateEntity(Entity entity);
        static void OnDestroyEntity(Entity entity);
        static void OnUpdateEntity(Entity entity, Timestep ts);
        static void OnCollisionBegin(Entity entityA, Entity entityB);
        static void OnCollisionEnd(Entity entityA, Entity entityB);
        static void OpenScene(const std::string& path);
        static MonoString* CreateString(const char* string);

        static Scene* GetSceneContext();
        static JPH::BodyInterface* GetBodyInterface();
        static Ref<ScriptClass> GetEntityClass(const std::string& className);
        static ScriptFieldMap& GetScriptFieldMap(Entity entity);
        static std::unordered_map<std::string, Ref<ScriptClass>> GetEntityClasses();
        static MonoImage* GetCoreAssemblyImage();
        static Ref<ScriptInstance> GetEntitySriptInstance(UUID entityID);
        static MonoObject* GetManagedInstance(UUID entityID);
    private:
        static void InitMono();
        static void ShutdownMono();

        static MonoObject* InstantiateClass(MonoClass* monoClass);
        friend class ScriptClass;
        friend class ScriptGlue;

    public:
        static void OnEntityPerceived(Entity perceiver, UUID targetID, PercaptionType method, const glm::vec3& position);
        static void OnEntityLost(Entity perceiver, UUID targetID, const glm::vec3& lastPosition);
        static void OnEntityForgotten(Entity perceiver, UUID targetID);
        
        static MonoObject* CreateBTActionInstance(const std::string& className);
        static MonoObject* CreateBTConditionInstance(const std::string& className);
        static MonoObject* CreateBTDecoratorInstance(const std::string& className);
        static MonoObject* CreateBTBlackboardInstance(const std::string& className);
        
        static void CallBTNodeOnStart(MonoObject* nodeInstance);
        static int CallBTNodeUpdate(MonoObject* nodeInstance);
        static void CallBTNodeOnFinished(MonoObject* nodeInstance);
        static void CallBTNodeOnAbort(MonoObject* nodeInstance);
        
        static bool CallBTConditionCheck(MonoObject* conditionInstance);
        static bool CallBTDecoratorCanExecute(MonoObject* decoratorInstance);
        static void CallBTDecoratorOnFinishedResult(MonoObject* decoratorInstance, NodeStatus& status);
        
        static void InitializeBTNode(MonoObject* nodeInstance, MonoObject* blackboardInstance, UUID entityID);

        struct BTParameterField
        {
            std::string Name;
            std::string DisplayName;
            ScriptFieldType Type;
            MonoClassField* Field;
            bool IsBlackboardKey;
            int BlackboardKeyType; // 0=Float, 1=Int, 2=Bool, 3=String
        };

        struct BTParameterInfo
        {
            std::string ClassName;
            MonoClass* MonoClass;
            std::vector<BTParameterField> Fields;
        };

        struct BTClassInfo
        {
            std::string ClassName;
            MonoClass* MonoClass;
            MonoMethod* OnStartMethod;
            MonoMethod* UpdateMethod;
            MonoMethod* OnFinishedMethod;
            MonoMethod* OnAbortMethod;
            MonoMethod* InitializeMethod;
            MonoMethod* GetParametersMethod;
            MonoMethod* SetParametersMethod;
            
            // Condition specific
            MonoMethod* CheckConditionMethod;
            
            // Decorator specific
            MonoMethod* CanExecuteMethod;
            MonoMethod* OnFinishedResultMethod;
        };

        static BTParameterInfo GetBTParameterInfo(const std::string& nodeClassName);
        static MonoObject* CreateBTParameterInstance(const std::string& nodeClassName);
        static void SerializeBTParameters(MonoObject* paramsInstance, YAML::Emitter& out);
        static void DeserializeBTParameters(MonoObject* paramsInstance, const YAML::Node& node);
        static void DrawBTParametersImGui(MonoObject* paramsInstance, HBlackboard* blackboard);

        static std::unordered_map<std::string, Ref<ScriptClass>> GetBTActionClasses();
        static std::unordered_map<std::string, Ref<ScriptClass>> GetBTConditionClasses();
        static std::unordered_map<std::string, Ref<ScriptClass>> GetBTDecoratorClasses();
        static std::unordered_map<std::string, Ref<ScriptClass>> GetBTBlackboardClasses();

        static std::unordered_map<std::string, BTClassInfo> s_BTActionClasses;
        static std::unordered_map<std::string, BTClassInfo> s_BTConditionClasses;
        static std::unordered_map<std::string, BTClassInfo> s_BTDecoratorClasses;
        static std::unordered_map<std::string, BTClassInfo> s_BTBlackboardClasses;
        static std::unordered_map<std::string, BTParameterInfo> s_BTParameterCache;
    };
}
