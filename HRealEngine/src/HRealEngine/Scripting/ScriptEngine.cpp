#include "HRpch.h"
#include "ScriptEngine.h"

#include <fstream>
#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>

#include "ScriptGlue.h"
#include "HRealEngine/Core/Components.h"
#include "HRealEngine/Core/Entity.h"

namespace HRealEngine
{
    //------------------------------------------------------------------ 
    //Mono Embedding for Game Engines
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
    //------------------------------------------------------------------
    
    struct ScriptEngineData
    {
        MonoDomain* RootDomain = nullptr;
        MonoDomain* AppDomain = nullptr;

        MonoAssembly* CoreAssembly = nullptr;
        MonoImage* CoreImage = nullptr;

        ScriptClass EntityClass;

        std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
        std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;

        Scene* SceneContext = nullptr;
    };
    static ScriptEngineData* s_Data = nullptr;
    
    void ScriptEngine::Init()
    {
        s_Data = new ScriptEngineData();
        InitMono();
        LoadAssembly("Resources/Scripts/HRealEngine-ScriptCore.dll");
        LoadAssemblyClasses(s_Data->CoreAssembly);

        ScriptGlue::RegisterComponents();
        ScriptGlue::RegisterFunctions();

        // Test
        s_Data->EntityClass = ScriptClass("HRealEngine", "Entity");
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

        s_Data->CoreAssembly = LoadCSharpAssembly(assemblyPath);
        //PrintAssemblyTypes(s_Data->CoreAssembly);
        s_Data->CoreImage = mono_assembly_get_image(s_Data->CoreAssembly);
    }

    void ScriptEngine::LoadAssemblyClasses(MonoAssembly* assembly)
    {
        s_Data->EntityClasses.clear();

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
        }
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
            instance->InvokeOnCreate();
        }
    }

    void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
    {
        UUID entityID = entity.GetUUID();
        Ref<ScriptInstance> instance = s_Data->EntityInstances[entityID];
        instance->InvokeOnUpdate(ts);
    }

    Scene* ScriptEngine::GetSceneContext()
    {
        return s_Data->SceneContext;
    }

    std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetEntityClasses()
    {
        return s_Data->EntityClasses;
    }

    MonoImage* ScriptEngine::GetCoreAssemblyImage()
    {
        return s_Data->CoreImage;
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
        //mono_domain_unload(s_Data->AppDomain);
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

    ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className)
        : m_ClassNamespace(classNamespace), m_ClassName(className)
    {
        m_MonoClass = mono_class_from_name(s_Data->CoreImage, m_ClassNamespace.c_str(), m_ClassName.c_str());
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
        m_Instance = m_ScriptClass->Instantiate();

        m_Constructor = m_ScriptClass->GetMethod(".ctor", 1);
        m_OnCreateMethod = m_ScriptClass->GetMethod("OnCreate", 0);
        m_OnUpdateMethod = m_ScriptClass->GetMethod("OnUpdate", 1);

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
}
