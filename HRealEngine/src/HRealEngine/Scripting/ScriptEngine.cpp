#include "HRpch.h"
#include "ScriptEngine.h"

#include <fstream>
#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>
#include <mono/metadata/tabledefs.h>

#include "ScriptGlue.h"
#include "HRealEngine/Core/Components.h"
#include "HRealEngine/Core/Entity.h"

namespace HRealEngine
{
    //------------------------------------------------------------------ 
    //Mono Embedding for Game Engines

    static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
    {
        { "System.Single",    ScriptFieldType::Float },
        { "System.Double",    ScriptFieldType::Double },
        { "System.Boolean",   ScriptFieldType::Bool },
        { "System.Char",      ScriptFieldType::Char },
        { "System.Byte",      ScriptFieldType::Byte },
        { "System.Int16",     ScriptFieldType::Short },
        { "System.Int32",     ScriptFieldType::Int },
        { "System.Int64",     ScriptFieldType::Long },
        { "System.UInt16",    ScriptFieldType::UShort },
        { "System.UInt32",    ScriptFieldType::UInt },
        { "System.UInt64",    ScriptFieldType::ULong },

        { "HRealEngine.Vector2",  ScriptFieldType::Vector2 },
        { "HRealEngine.Vector3",  ScriptFieldType::Vector3 },
        { "HRealEngine.Vector4",  ScriptFieldType::Vector4 },

        { "HRealEngine.Entity",   ScriptFieldType::Entity }
    };
    
    static char* ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize)
    {
        std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
    
        if (!stream)
        {
            // Failed to open the file
            return nullptr;
        }

        std::streampos end = stream.tellg();
        stream.seekg(0, std::ios::beg);
        uint32_t size = end - stream.tellg();
    
        if (size == 0)
        {
            // File is empty
            return nullptr;
        }

        char* buffer = new char[size];
        stream.read((char*)buffer, size);
        stream.close();

        *outSize = size;
        return buffer;
    }
    static MonoAssembly* LoadCSharpAssembly(const std::filesystem::path& assemblyPath)
    {
        uint32_t fileSize = 0;
        char* fileData = ReadBytes(assemblyPath, &fileSize);

        // NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
        MonoImageOpenStatus status;
        MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

        if (status != MONO_IMAGE_OK)
        {
            const char* errorMessage = mono_image_strerror(status);
            // Log some error message using the errorMessage data
            return nullptr;
        }

        std::string pathString = assemblyPath.string();
        MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
        mono_image_close(image);
    
        // Don't forget to free the file data
        delete[] fileData;

        return assembly;
    }

    void PrintAssemblyTypes(MonoAssembly* assembly)
    {
        MonoImage* image = mono_assembly_get_image(assembly);
        const MonoTableInfo* typeDefTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
        uint32_t typeCount = mono_image_get_table_rows(image, MONO_TABLE_TYPEDEF);

        for (uint32_t i = 0; i < typeCount; i++)
        {
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(typeDefTable, i, cols, MONO_TYPEDEF_SIZE);

            const char* typeNamespace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
            const char* typeName = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
            printf("%s.%s", typeNamespace, typeName);
        }
    }

    ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
    {
        std::string typeName = mono_type_get_name(monoType);

        auto it = s_ScriptFieldTypeMap.find(typeName);
        if (it == s_ScriptFieldTypeMap.end())
        {
            LOG_CORE_ERROR("MonoTypeToScriptFieldType: Unknown type '{0}'", typeName);
            return ScriptFieldType::None;
        }
        return it->second;
    }
    //------------------------------------------------------------------
    
    struct ScriptEngineData
    {
        MonoDomain* RootDomain = nullptr;
        MonoDomain* AppDomain = nullptr;

        MonoAssembly* CoreAssembly = nullptr;
        MonoImage* CoreImage = nullptr;

        MonoAssembly* AppAssembly = nullptr;
        MonoImage* AppImage = nullptr;

        ScriptClass EntityClass;

        std::filesystem::path CoreAssemblyFilePath;
        std::filesystem::path AppAssemblyFilePath;

        std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
        std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;
        std::unordered_map<UUID, ScriptFieldMap> EntityScriptFieldMaps;

        Scene* SceneContext = nullptr;
    };
    static ScriptEngineData* s_Data = nullptr;
    
    void ScriptEngine::Init()
    {
        s_Data = new ScriptEngineData();
        InitMono();
        LoadAssembly("Resources/Scripts/HRealEngine-ScriptCore.dll");
        LoadAppAssembly("SandboxProject/Assets/Scripts/Binaries/Sandbox.dll");
        LoadAssemblyClasses(/*s_Data->CoreAssembly*/);

        ScriptGlue::RegisterComponents();//
        ScriptGlue::RegisterFunctions();

        // Test
        s_Data->EntityClass = ScriptClass("HRealEngine", "Entity", true);
        /*MonoObject* instance = s_Data->EntityClass.Instantiate();

        MonoMethod* printIntFunc = s_Data->EntityClass.GetMethod("PrintInt", 1);
        int intValue = 123;
        void* param = &intValue;
        s_Data->EntityClass.InvokeMethod(instance, printIntFunc, &param);
        MonoMethod* printIntsFunc = s_Data->EntityClass.GetMethod("PrintInts", 2);
        int intValue2 = 1;
        void* params[2]
        {
            &intValue, &intValue2
        };
        s_Data->EntityClass.InvokeMethod(instance, printIntsFunc, params);

        MonoString* str = mono_string_new(s_Data->AppDomain, "Hello from C++!");
        MonoMethod* printStringFunc = s_Data->EntityClass.GetMethod("PrintCustomMessage", 1);
        void* stringParam = str;
        s_Data->EntityClass.InvokeMethod(instance, printStringFunc, &stringParam);*/
    }

    void ScriptEngine::Shutdown()
    {
        ShutdownMono();
        delete s_Data;
    }

    void ScriptEngine::LoadAssembly(const std::filesystem::path& assemblyPath)
    {
        s_Data->AppDomain = mono_domain_create_appdomain("HRealEngineAppDomain", nullptr);
        mono_domain_set(s_Data->AppDomain, true);

        s_Data->CoreAssemblyFilePath = assemblyPath;
        
        s_Data->CoreAssembly = LoadCSharpAssembly(assemblyPath);
        //PrintAssemblyTypes(s_Data->CoreAssembly);
        s_Data->CoreImage = mono_assembly_get_image(s_Data->CoreAssembly);
    }

    void ScriptEngine::LoadAppAssembly(const std::filesystem::path& assemblyPath)
    {
        s_Data->AppAssemblyFilePath = assemblyPath;
        
        s_Data->AppAssembly = LoadCSharpAssembly(assemblyPath);
        auto appAsemb = s_Data->AppAssembly;
        s_Data->AppImage = mono_assembly_get_image(s_Data->AppAssembly);
        auto appImg = s_Data->AppImage;
    }

    void ScriptEngine::LoadAssemblyClasses(/*MonoAssembly* assembly*/)
    {
        /*s_Data->EntityClasses.clear();

        MonoImage* image = mono_assembly_get_image(assembly);
        const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
        int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
        MonoClass* entityClass = mono_class_from_name(image, "HRealEngine", "Entity");

        for (int32_t i = 0; i < numTypes; i++)
        {
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

            const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
            const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
            std::string fullName;
            if (strlen(nameSpace) != 0)
                fullName = fmt::format("{}.{}", nameSpace, name);
            else
                fullName = name;

            MonoClass* monoClass = mono_class_from_name(image, nameSpace, name);

            if (monoClass == entityClass)
                continue;

            bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
            if (isEntity)
                s_Data->EntityClasses[fullName] = CreateRef<ScriptClass>(nameSpace, name);
        }*/
        s_Data->EntityClasses.clear();
        const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_Data->AppImage, MONO_TABLE_TYPEDEF);
        int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
        MonoClass* entityClass = mono_class_from_name(s_Data->CoreImage, "HRealEngine", "Entity");
        for (int32_t i = 0; i < numTypes; i++)
        {
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

            const char* nameSpace = mono_metadata_string_heap(s_Data->AppImage, cols[MONO_TYPEDEF_NAMESPACE]);
            const char* className = mono_metadata_string_heap(s_Data->AppImage, cols[MONO_TYPEDEF_NAME]);
            std::string fullName;
            if (strlen(nameSpace) != 0)
                fullName = fmt::format("{}.{}", nameSpace, className);
            else
                fullName = className;

            MonoClass* monoClass = mono_class_from_name(s_Data->AppImage, nameSpace, className);

            if (monoClass == entityClass)
                continue;

            bool bIsEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
            /*if (bIsEntity)
                s_Data->EntityClasses[fullName] = CreateRef<ScriptClass>(nameSpace, className);*/
            if (!bIsEntity)
                continue;
            Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className);
            s_Data->EntityClasses[fullName] = scriptClass;

            int fieldCount = mono_class_num_fields(monoClass);
            LOG_CORE_WARN("Class {0} has {1} fields", fullName, fieldCount);
            void* iterForMono = nullptr;
            while (MonoClassField* field = mono_class_get_fields(monoClass, &iterForMono))
            {
                const  char* fieldName = mono_field_get_name(field);
                uint32_t flags = mono_field_get_flags(field);
                if (flags & FIELD_ATTRIBUTE_PUBLIC)
                {
                    MonoType* type = mono_field_get_type(field);
                    ScriptFieldType scriptFieldType = MonoTypeToScriptFieldType(type);
                    LOG_CORE_WARN("  {} ({})", fieldName, ScriptFieldTypeToString(scriptFieldType));
                    scriptClass->m_Fields[fieldName] = { scriptFieldType, fieldName, field };
                }
            }
        }
        auto& entityClasses = s_Data->EntityClasses;
    }

    void ScriptEngine::ReloadAssembly()
    {
        mono_domain_set(mono_get_root_domain(), false);
        mono_domain_unload(s_Data->AppDomain);

        LoadAssembly(s_Data->CoreAssemblyFilePath);
        LoadAppAssembly(s_Data->AppAssemblyFilePath);
        LoadAssemblyClasses();
        ScriptGlue::RegisterComponents();
        
        s_Data->EntityClass = ScriptClass("HRealEngine", "Entity", true);
    }

    void ScriptEngine::OnRuntimeStart(Scene* scene)
    {
        s_Data->SceneContext = scene;
    }

    void ScriptEngine::OnRuntimeStop()
    {
        s_Data->SceneContext = nullptr; 
    }

    bool ScriptEngine::IsEntityClassExist(const std::string& className)
    {
        return s_Data->EntityClasses.find(className) != s_Data->EntityClasses.end();
    }

    void ScriptEngine::OnCreateEntity(Entity entity)
    {
        const auto& scriptComponent = entity.GetComponent<ScriptComponent>();
        if (ScriptEngine::IsEntityClassExist(scriptComponent.ClassName))
        {
            Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_Data->EntityClasses[scriptComponent.ClassName], entity);
            s_Data->EntityInstances[entity.GetUUID()] = instance;
            if (s_Data->EntityScriptFieldMaps.find(entity.GetUUID()) != s_Data->EntityScriptFieldMaps.end())
            {
                const ScriptFieldMap& fieldMap = s_Data->EntityScriptFieldMaps.at(entity.GetUUID());
                for (const auto& [name, field] : fieldMap)
                    instance->SetFieldValueInternal(name, field.m_Buffer);
            }
            instance->InvokeOnCreate();
        }
    }

    void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
    {
        UUID entityID = entity.GetUUID();
        Ref<ScriptInstance> instance = s_Data->EntityInstances[entityID];
        instance->InvokeOnUpdate((float)ts);
    }

    Scene* ScriptEngine::GetSceneContext()
    {
        return s_Data->SceneContext;
    }

    Ref<ScriptClass> ScriptEngine::GetEntityClass(const std::string& className)
    {
        if (s_Data->EntityClasses.find(className) == s_Data->EntityClasses.end())
            return nullptr;
        return s_Data->EntityClasses.at(className);
    }

    ScriptFieldMap& ScriptEngine::GetScriptFieldMap(Entity entity)
    {
        HREALENGINE_CORE_DEBUGBREAK(entity);
        return s_Data->EntityScriptFieldMaps[entity.GetUUID()];
    }

    std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetEntityClasses()
    {
        return s_Data->EntityClasses;
    }

    MonoImage* ScriptEngine::GetCoreAssemblyImage()
    {
        return s_Data->CoreImage;
    }

    Ref<ScriptInstance> ScriptEngine::GetEntitySriptInstance(UUID entityID)
    {
        auto it = s_Data->EntityInstances.find(entityID);
        if (it == s_Data->EntityInstances.end())
            return nullptr;
        return it->second;
    }

    MonoObject* ScriptEngine::GetManagedInstance(UUID entityID)
    {
        HREALENGINE_CORE_DEBUGBREAK(s_Data->EntityInstances.find(entityID) != s_Data->EntityInstances.end());
        return s_Data->EntityInstances.at(entityID)->GetManagedObject();
    }

    void ScriptEngine::InitMono()
    {
        mono_set_assemblies_path("mono/lib/mono/4.5");

        MonoDomain* rootDomain = mono_jit_init("HRealEngineJITRuntime");
        HREALENGINE_CORE_DEBUGBREAK(domain, "Failed to initialize Mono JIT Runtime!");

        s_Data->RootDomain = rootDomain;
    }

    void ScriptEngine::ShutdownMono()
    {
        mono_domain_set(mono_get_root_domain(), false);
        mono_domain_unload(s_Data->AppDomain);
        s_Data->AppDomain = nullptr;
        mono_jit_cleanup(s_Data->RootDomain);
        s_Data->RootDomain = nullptr;
    }

    MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
    {
        MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
        mono_runtime_object_init(instance);
        return instance;
    }

    //---------------- ScriptClass ----------------

    ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool bIsCore)
        : m_ClassNamespace(classNamespace), m_ClassName(className)
    {
        m_MonoClass = mono_class_from_name(bIsCore ? s_Data->CoreImage : s_Data->AppImage, m_ClassNamespace.c_str(), m_ClassName.c_str());
    }
    MonoObject* ScriptClass::Instantiate()
    {
        return ScriptEngine::InstantiateClass(m_MonoClass);
    }
    MonoMethod* ScriptClass::GetMethod(const std::string& methodName, int paramCount)
    {
        return mono_class_get_method_from_name(m_MonoClass, methodName.c_str(), paramCount);
    }
    MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
    {
        return mono_runtime_invoke(method, instance, params, nullptr);
    }

    //---------------- ScriptInstance ----------------

    ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity) : m_ScriptClass(scriptClass)
    {
        m_Instance = scriptClass->Instantiate();

        m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);
        m_OnCreateMethod = scriptClass->GetMethod("OnCreate", 0);
        m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);

        {
            UUID entityID = entity.GetUUID();
            void* param = &entityID;
            m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
        }
    }

    void ScriptInstance::InvokeOnCreate()
    {
        if (m_OnCreateMethod)
            m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod);
    }

    void ScriptInstance::InvokeOnUpdate(Timestep ts)
    {
        if (m_OnUpdateMethod)
        {
            float time = (float)ts;
            void* param = &time;
            m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, &param);
        }
    }

    bool ScriptInstance::GetFieldValueInternal(const std::string& name, void* outValue)
    {
        const auto& fields = m_ScriptClass->GetFields();
        auto it = fields.find(name);
        if (it == fields.end())
            return false;
        const ScriptField& field = it->second;
        mono_field_get_value(m_Instance, field.ClassField, outValue);
        return true;
    }

    bool ScriptInstance::SetFieldValueInternal(const std::string& name, const void* value)
    {
        const auto& fields = m_ScriptClass->GetFields();
        auto it = fields.find(name);
        if (it == fields.end())
            return false;
        const ScriptField& field = it->second;
        mono_field_set_value(m_Instance, field.ClassField, (void*)value);
        return true;
    }
}
