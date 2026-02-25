#include "HRpch.h"
#include "ScriptEngine.h"

#include <fstream>
#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>
#include <mono/metadata/tabledefs.h>

#include "ScriptGlue.h"
#include "HRealEngine/Core/Components.h"
#include "HRealEngine/Core/Entity.h"

#include "FileWatch.hpp"
#include "imgui_internal.h"
#include "HRealEngine/Core/Application.h"
#include "HRealEngine/Core/Buffer.h"
#include "HRealEngine/Core/FileSystem.h"
#include "HRealEngine/Core/Input.h"
#include "HRealEngine/Project/Project.h"

namespace JPH
{
    class BodyInterface;
}

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

        { "HRealEngine.Entity",   ScriptFieldType::Entity },
        { "System.String",    ScriptFieldType::String } 
    };

    static MonoClassField* FindFieldInHierarchy(MonoClass* klass, const char* fieldName)
    {
        MonoClass* current = klass;
        while (current != nullptr)
        {
            MonoClassField* field = mono_class_get_field_from_name(current, fieldName);
            if (field)
                return field;
            current = mono_class_get_parent(current);
        }
        return nullptr;
    }
    static MonoMethod* FindMethodInHierarchy(MonoClass* klass, const char* methodName, int paramCount)
    {
        MonoClass* current = klass;
        while (current != nullptr)
        {
            MonoMethod* method = mono_class_get_method_from_name(current, methodName, paramCount);
            if (method)
                return method;
            current = mono_class_get_parent(current);
        }
        return nullptr;
    }
    
    /*static char* ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize)
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
    }*/
    static MonoAssembly* LoadCSharpAssembly(const std::filesystem::path& assemblyPath)
    {
        /*uint32_t fileSize = 0;
        char* fileData = ReadBytes(assemblyPath, &fileSize);*/
        ScopedBuffer fileData = FileSystem::ReadFileBinary(assemblyPath);

        // NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
        MonoImageOpenStatus status;
        //MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);
        MonoImage* image = mono_image_open_from_data_full(fileData.As<char>(), static_cast<uint32_t>(fileData.Size()), 1, &status, 0);

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
        //delete[] fileData;

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

        Scope<filewatch::FileWatch<std::string>> AppAssemblyWatcher;
        bool bAssemblyReloadPending = false;

        Scene* SceneContext = nullptr;
        JPH::BodyInterface* body_interface;
    };
    static ScriptEngineData* s_Data = nullptr;
    
    void ScriptEngine::Init()
    {
        if (IsInitialized())
            return;
        auto scriptDir = Project::GetProjectDirectory() / Project::GetActive()->GetConfig().ScriptModulePath;
        LOG_CORE_WARN("AppAssemblyPath = {}", scriptDir.string());
        LOG_CORE_WARN("  exists? {}", std::filesystem::exists(scriptDir) ? "YES" : "NO");
        if (!std::filesystem::exists(scriptDir))
        {
            LOG_CORE_ERROR("Script assembly does not exist: {}", scriptDir.string());
            return;
        }
        
        s_Data = new ScriptEngineData();
        InitMono();
        auto corePath = Application::Get().GetSpecification().EditorAssetsPath / "scriptcore/HRealEngine-ScriptCore.dll";
        bool bstatus = LoadAssembly(corePath);
        //bool bstatus = LoadAssembly("Resources/Scripts/HRealEngine-ScriptCore.dll");
        if (!bstatus)
        {
            LOG_CORE_ERROR("Failed to load HRealEngine-ScriptCore.dll");
            return;
        }

        bstatus = LoadAppAssembly(scriptDir);
        if (!bstatus)
        {
            LOG_CORE_ERROR("Failed to load application script assembly");
            return;
        }
        LoadAssemblyClasses();

        ScriptGlue::RegisterComponents();
        ScriptGlue::RegisterFunctions();

        s_Data->EntityClass = ScriptClass("HRealEngine", "Entity", true);
    }

    void ScriptEngine::Shutdown()
    {
        if (!s_Data)
            return;

        s_Data->AppAssemblyWatcher.reset();
        s_Data->bAssemblyReloadPending = false;

        s_Data->EntityInstances.clear();
        s_Data->EntityScriptFieldMaps.clear();
        s_Data->EntityClasses.clear();
        s_Data->SceneContext = nullptr;
        s_Data->body_interface = nullptr;

        ShutdownMono();

        delete s_Data;
        s_Data = nullptr;
    }

    bool ScriptEngine::IsInitialized()
    {
        return s_Data != nullptr;
    }

    bool ScriptEngine::LoadAssembly(const std::filesystem::path& assemblyPath)
    {
        s_Data->AppDomain = mono_domain_create_appdomain("HRealEngineAppDomain", nullptr);
        mono_domain_set(s_Data->AppDomain, true);

        s_Data->CoreAssemblyFilePath = assemblyPath;
        
        s_Data->CoreAssembly = LoadCSharpAssembly(assemblyPath);
        if (!s_Data->CoreAssembly)
            return false;
        //PrintAssemblyTypes(s_Data->CoreAssembly);
        s_Data->CoreImage = mono_assembly_get_image(s_Data->CoreAssembly);
        return true;
    }

    bool ScriptEngine::LoadAppAssembly(const std::filesystem::path& assemblyPath)
    {
        s_Data->AppAssemblyFilePath = assemblyPath;
        
        s_Data->AppAssembly = LoadCSharpAssembly(assemblyPath);
        if (!s_Data->AppAssembly)
            return false;
        s_Data->AppImage = mono_assembly_get_image(s_Data->AppAssembly);

        s_Data->AppAssemblyWatcher = CreateScope<filewatch::FileWatch<std::string>>(assemblyPath.string(),/*OnAppAssemblyFileSystemEvent*/ [](const std::string& path, const filewatch::Event change_type)
        {
            if (!s_Data->bAssemblyReloadPending && change_type == filewatch::Event::modified)
            {
                s_Data->bAssemblyReloadPending = true;
                Application::Get().SubmitToMainThread([]()
                {
                    s_Data->AppAssemblyWatcher.reset();
                    ScriptEngine::ReloadAssembly();
                });
            }
        });
        s_Data->bAssemblyReloadPending = false;
        return true;
    }

    // Static member definitions
    std::unordered_map<std::string, ScriptEngine::BTClassInfo> ScriptEngine::s_BTActionClasses;
    std::unordered_map<std::string, ScriptEngine::BTClassInfo> ScriptEngine::s_BTConditionClasses;
    std::unordered_map<std::string, ScriptEngine::BTClassInfo> ScriptEngine::s_BTDecoratorClasses;
    std::unordered_map<std::string, ScriptEngine::BTClassInfo> ScriptEngine::s_BTBlackboardClasses;
    std::unordered_map<std::string, ScriptEngine::BTParameterInfo> ScriptEngine::s_BTParameterCache;
    
    void ScriptEngine::LoadAssemblyClasses()
    {
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

        // Clear BT class maps
        s_BTActionClasses.clear();
        s_BTConditionClasses.clear();
        s_BTDecoratorClasses.clear();
        s_BTBlackboardClasses.clear();
        s_BTParameterCache.clear();

        // Get base classes
        MonoClass* btActionBase = mono_class_from_name(s_Data->CoreImage, "HRealEngine.BehaviorTree", "BTActionNode");
        MonoClass* btConditionBase = mono_class_from_name(s_Data->CoreImage, "HRealEngine.BehaviorTree", "BTCondition");
        MonoClass* btDecoratorBase = mono_class_from_name(s_Data->CoreImage, "HRealEngine.BehaviorTree", "BTDecorator");
        MonoClass* btBlackboardBase = mono_class_from_name(s_Data->CoreImage, "HRealEngine.BehaviorTree", "BTBlackboard");
        
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

            // Skip abstract classes using flags
            uint32_t classFlags = mono_class_get_flags(monoClass);
            if (classFlags & TYPE_ATTRIBUTE_ABSTRACT)
                continue;

            // Check for BT Action
        if (btActionBase && mono_class_is_subclass_of(monoClass, btActionBase, false) && monoClass != btActionBase)
        {
            BTClassInfo info;
            info.ClassName = fullName;
            info.MonoClass = monoClass;
            info.OnStartMethod = mono_class_get_method_from_name(monoClass, "OnStart", 0);
            info.UpdateMethod = mono_class_get_method_from_name(monoClass, "Update", 0);
            info.OnFinishedMethod = mono_class_get_method_from_name(monoClass, "OnFinished", 0);
            info.OnAbortMethod = mono_class_get_method_from_name(monoClass, "OnAbort", 0);
            info.InitializeMethod = mono_class_get_method_from_name(monoClass, "Initialize", 2);
            info.GetParametersMethod = mono_class_get_method_from_name(monoClass, "GetParameters", 0);
            info.SetParametersMethod = mono_class_get_method_from_name(monoClass, "SetParameters", 1);
            
            s_BTActionClasses[fullName] = info;
            LOG_CORE_INFO("Registered BT Action: {}", fullName);
        }
        
        // Check for BT Condition
        if (btConditionBase && mono_class_is_subclass_of(monoClass, btConditionBase, false) && monoClass != btConditionBase)
        {
            BTClassInfo info;
            info.ClassName = fullName;
            info.MonoClass = monoClass;
            info.OnStartMethod = mono_class_get_method_from_name(monoClass, "OnStart", 0);
            info.CheckConditionMethod = mono_class_get_method_from_name(monoClass, "CheckCondition", 0);
            info.OnFinishedMethod = mono_class_get_method_from_name(monoClass, "OnFinished", 0);
            info.OnAbortMethod = mono_class_get_method_from_name(monoClass, "OnAbort", 0);
            info.InitializeMethod = mono_class_get_method_from_name(monoClass, "Initialize", 2);
            info.GetParametersMethod = mono_class_get_method_from_name(monoClass, "GetParameters", 0);
            info.SetParametersMethod = mono_class_get_method_from_name(monoClass, "SetParameters", 1);
            
            s_BTConditionClasses[fullName] = info;
            LOG_CORE_INFO("Registered BT Condition: {}", fullName);
        }
        
        // Check for BT Decorator
        if (btDecoratorBase && mono_class_is_subclass_of(monoClass, btDecoratorBase, false) && monoClass != btDecoratorBase)
        {
            BTClassInfo info;
            info.ClassName = fullName;
            info.MonoClass = monoClass;
            info.OnStartMethod = mono_class_get_method_from_name(monoClass, "OnStart", 0);
            info.CanExecuteMethod = mono_class_get_method_from_name(monoClass, "CanExecute", 0);
            info.OnFinishedResultMethod = mono_class_get_method_from_name(monoClass, "OnFinishedResult", 1);
            info.OnFinishedMethod = mono_class_get_method_from_name(monoClass, "OnFinished", 0);
            info.OnAbortMethod = mono_class_get_method_from_name(monoClass, "OnAbort", 0);
            info.InitializeMethod = mono_class_get_method_from_name(monoClass, "Initialize", 2);
            info.GetParametersMethod = mono_class_get_method_from_name(monoClass, "GetParameters", 0);
            info.SetParametersMethod = mono_class_get_method_from_name(monoClass, "SetParameters", 1);
            
            s_BTDecoratorClasses[fullName] = info;
            LOG_CORE_INFO("Registered BT Decorator: {}", fullName);
        }
        
        // Check for BT Blackboard
        if (btBlackboardBase && mono_class_is_subclass_of(monoClass, btBlackboardBase, false) && monoClass != btBlackboardBase)
        {
            BTClassInfo info;
            info.ClassName = fullName;
            info.MonoClass = monoClass;
            
            s_BTBlackboardClasses[fullName] = info;
            LOG_CORE_INFO("Registered BT Blackboard: {}", fullName);
        }
        }
    }

    void ScriptEngine::InitCSharpProject()
    {
        Init();
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

    void ScriptEngine::SetBodyInterface(JPH::BodyInterface* bodyInterface)
    {
        if (s_Data)
            s_Data->body_interface = bodyInterface;
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
                {
                    if (field.Field.Type == ScriptFieldType::String)
                    {
                        MonoString* monoString = mono_string_new(s_Data->AppDomain, field.m_StringStorage.c_str());
                        instance->SetFieldValueInternal(name, monoString);
                    }
                    else
                        instance->SetFieldValueInternal(name, field.m_Buffer);
                }
            }
            instance->InvokeBeginPlay();
        }
    }

    void ScriptEngine::OnDestroyEntity(Entity entity)
    {
        UUID entityID = entity.GetUUID();
        auto it = s_Data->EntityInstances.find(entityID);
        if (it != s_Data->EntityInstances.end() && it->second)
        {
            it->second->InvokeOnDestroy();
            s_Data->EntityInstances.erase(it);
        }
        s_Data->EntityScriptFieldMaps.erase(entityID);
    }

    void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
    {
        UUID entityID = entity.GetUUID();
        if (s_Data->EntityInstances.find(entityID) == s_Data->EntityInstances.end())
        {
            LOG_CORE_WARN("Script instance for entity {0} not found!", (uint64_t)entityID);
            return;
        }
        Ref<ScriptInstance> instance = s_Data->EntityInstances[entityID];
        instance->InvokeTick((float)ts);
    }

    void ScriptEngine::OnCollisionBegin(Entity entityA, Entity entityB)
    {
        UUID idA = entityA.GetUUID();
        UUID idB = entityB.GetUUID();
        Ref<ScriptInstance> instanceA = s_Data->EntityInstances[idA];
        if (instanceA)
            instanceA->InvokeOnCollisionEnter(idB);
        Ref<ScriptInstance> instanceB = s_Data->EntityInstances[idB];
        if (instanceB)
            instanceB->InvokeOnCollisionEnter(idA);
    }

    void ScriptEngine::OnCollisionEnd(Entity entityA, Entity entityB)
    {
        UUID idA = entityA.GetUUID();
        UUID idB = entityB.GetUUID();
        Ref<ScriptInstance> instanceA = s_Data->EntityInstances[idA];
        if (instanceA)
            instanceA->InvokeOnCollisionExit(idB);
        Ref<ScriptInstance> instanceB = s_Data->EntityInstances[idB];
        if (instanceB)
            instanceB->InvokeOnCollisionExit(idA);
    }

    void ScriptEngine::OpenScene(const std::string& path)
    {
        LOG_CORE_INFO("Opening scene {0} from script", path);
        auto editorAssetManager = Project::GetActive()->GetEditorAssetManager();
        AssetHandle sceneHandle = editorAssetManager->GetHandleFromPath(path);
        LOG_CORE_INFO("Got scene handle {0} from path {1}", static_cast<uint64_t>(sceneHandle), path);
        if (sceneHandle)
        {
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
            Input::SetCursorMode(CursorMode::Locked);
            SceneChangeEvent event(static_cast<uint64_t>(sceneHandle));
            Application::Get().OnEvent(event);
        }
    }

    MonoString* ScriptEngine::CreateString(const char* string)
    {
        return mono_string_new(s_Data->AppDomain, string);
    }

    Scene* ScriptEngine::GetSceneContext()
    {
        return s_Data->SceneContext;
    }

    JPH::BodyInterface* ScriptEngine::GetBodyInterface()
    {
        return s_Data->body_interface;
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
        HREALENGINE_CORE_DEBUGBREAK(rootDomain, "Failed to initialize Mono JIT Runtime!");
        s_Data->RootDomain = rootDomain;
    }

    void ScriptEngine::ShutdownMono()
    {
        /*mono_domain_set(mono_get_root_domain(), false);
        mono_domain_unload(s_Data->AppDomain);
        s_Data->AppDomain = nullptr;
        mono_jit_cleanup(s_Data->RootDomain);
        s_Data->RootDomain = nullptr;*/
        if (!s_Data)
            return;

        if (s_Data->AppDomain)
        {
            mono_domain_set(mono_get_root_domain(), false);
            mono_domain_unload(s_Data->AppDomain);
            s_Data->AppDomain = nullptr;
        }

        if (s_Data->RootDomain)
        {
            mono_jit_cleanup(s_Data->RootDomain);
            s_Data->RootDomain = nullptr;
        }
    }

    MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
    {
        MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
        MonoObject* exception = nullptr;
        mono_runtime_invoke(mono_class_get_method_from_name(monoClass, ".ctor", 0), instance, nullptr, &exception);
        if (exception)
        {
            MonoString* exceptionMessage = mono_object_to_string(exception, nullptr);
            char* exceptionChars = mono_string_to_utf8(exceptionMessage);
            LOG_CORE_ERROR("Failed to instantiate class: {0}", exceptionChars);
            mono_free(exceptionChars);
        }
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
        m_BeginPlayMethod = scriptClass->GetMethod("BeginPlay", 0);
        m_TickMethod = scriptClass->GetMethod("Tick", 1);
        
        m_OnCollisionEnterMethod = scriptClass->GetMethod("OnCollisionEnter", 1);
        m_OnCollisionExitMethod = scriptClass->GetMethod("OnCollisionExit", 1);
        m_OnDestroyMethod = scriptClass->GetMethod("OnDestroy", 0);

        {
            UUID entityID = entity.GetUUID();
            void* param = &entityID;
            m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
        }
    }

    void ScriptInstance::InvokeBeginPlay()
    {
        if (m_BeginPlayMethod)
            m_ScriptClass->InvokeMethod(m_Instance, m_BeginPlayMethod);
    }

    void ScriptInstance::InvokeOnDestroy()
    {
        if (m_OnDestroyMethod)
            m_ScriptClass->InvokeMethod(m_Instance, m_OnDestroyMethod);
    }

    void ScriptInstance::InvokeTick(Timestep ts)
    {
        if (m_TickMethod)
        {
            float time = (float)ts;
            void* param = &time;
            m_ScriptClass->InvokeMethod(m_Instance, m_TickMethod, &param);
        }
    }

    void ScriptInstance::InvokeOnCollisionEnter(UUID otherID)
    {
        if (m_OnCollisionEnterMethod)
        {
            void* param = &otherID;
            m_ScriptClass->InvokeMethod(m_Instance, m_OnCollisionEnterMethod, &param);
        }
    }

    void ScriptInstance::InvokeOnCollisionExit(UUID otherID)
    {
        if (m_OnCollisionExitMethod)
        {
            void* param = &otherID;
            m_ScriptClass->InvokeMethod(m_Instance, m_OnCollisionExitMethod, &param);
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


    void ScriptEngine::OnEntityPerceived(Entity perceiver, UUID targetID, PercaptionType method, const glm::vec3& position)
    {
        if (!perceiver.HasComponent<ScriptComponent>())
            return;
    
        Ref<ScriptInstance> instance = GetEntitySriptInstance(perceiver.GetUUID());
        if (!instance)
            return;
    
        MonoMethod* onPerceived = instance->GetScriptClass()->GetMethod("OnEntityPerceived", 3);
        if (!onPerceived)
            return;
    
        uint64_t targetIDVal = targetID;
        int methodVal = (int)method;
        glm::vec3 pos = position;
        void* params[3] = { &targetIDVal, &methodVal, &pos };
        instance->GetScriptClass()->InvokeMethod(instance->GetManagedObject(), onPerceived, params);
    }

    void ScriptEngine::OnEntityLost(Entity perceiver, UUID targetID, const glm::vec3& lastPosition)
    {
        if (!perceiver.HasComponent<ScriptComponent>())
            return;
    
        Ref<ScriptInstance> instance = GetEntitySriptInstance(perceiver.GetUUID());
        if (!instance)
            return;
    
        MonoMethod* onLost = instance->GetScriptClass()->GetMethod("OnEntityLost", 2);
        if (!onLost)
            return;
    
        uint64_t targetIDVal = targetID;
        glm::vec3 pos = lastPosition;
        void* params[2] = { &targetIDVal, &pos };
        instance->GetScriptClass()->InvokeMethod(instance->GetManagedObject(), onLost, params);
    }

    void ScriptEngine::OnEntityForgotten(Entity perceiver, UUID targetID)
    {
        if (!perceiver.HasComponent<ScriptComponent>()) 
            return;
    
        Ref<ScriptInstance> instance = GetEntitySriptInstance(perceiver.GetUUID());
        if (!instance) 
            return;
    
        MonoMethod* onForgotten = instance->GetScriptClass()->GetMethod("OnEntityForgotten", 1);
        if (!onForgotten) 
            return;
    
        uint64_t targetIDVal = targetID;
        void* params[1] = { &targetIDVal };
        instance->GetScriptClass()->InvokeMethod(instance->GetManagedObject(), onForgotten, params);
    }

    MonoObject* ScriptEngine::CreateBTActionInstance(const std::string& className)
    {
        if (s_BTActionClasses.find(className) == s_BTActionClasses.end())
        {
            LOG_CORE_ERROR("BT Action class not found: {}", className);
            return nullptr;
        }
        
        return InstantiateClass(s_BTActionClasses[className].MonoClass);
    }

    MonoObject* ScriptEngine::CreateBTConditionInstance(const std::string& className)
    {
        if (s_BTConditionClasses.find(className) == s_BTConditionClasses.end())
        {
            LOG_CORE_ERROR("BT Condition class not found: {}", className);
            return nullptr;
        }
        
        return InstantiateClass(s_BTConditionClasses[className].MonoClass);
    }

    MonoObject* ScriptEngine::CreateBTDecoratorInstance(const std::string& className)
    {
        if (s_BTDecoratorClasses.find(className) == s_BTDecoratorClasses.end())
        {
            LOG_CORE_ERROR("BT Decorator class not found: {}", className);
            return nullptr;
        }
        
        return InstantiateClass(s_BTDecoratorClasses[className].MonoClass);
    }

    MonoObject* ScriptEngine::CreateBTBlackboardInstance(const std::string& className)
    {
        if (s_BTBlackboardClasses.find(className) == s_BTBlackboardClasses.end())
        {
            LOG_CORE_ERROR("BT Blackboard class not found: {}", className);
            return nullptr;
        }
        
        return InstantiateClass(s_BTBlackboardClasses[className].MonoClass);
    }

    void ScriptEngine::InitializeBTNode(MonoObject* nodeInstance, MonoObject* blackboardInstance, UUID entityID)
    {
        if (!nodeInstance)
            return;

        MonoClass* klass = mono_object_get_class(nodeInstance);
        //MonoMethod* initMethod = mono_class_get_method_from_name(klass, "Initialize", 2);
        MonoMethod* initMethod = FindMethodInHierarchy(klass, "Initialize", 2);
        if (initMethod)
        {
            Scene* scene = ScriptEngine::GetSceneContext();
            if (!scene)
            {
                LOG_CORE_ERROR("InitializeBTNode: Scene context is null!");
                return;
            }
        
            Entity verifyEntity = scene->GetEntityByUUID(entityID);
            if (!verifyEntity)
            {
                LOG_CORE_ERROR("InitializeBTNode: Invalid entity UUID: {}", (uint64_t)entityID);
                return;
            }
            
            void* args[2];
            args[0] = blackboardInstance;
            args[1] = &entityID;
        
            LOG_CORE_INFO("Calling Initialize method with blackboard: {}, entityID: {}", (void*)blackboardInstance, (uint64_t)entityID);
        
            MonoObject* exception = nullptr;
            mono_runtime_invoke(initMethod, nodeInstance, args, &exception);
        
            if (exception)
            {
                MonoString* exMsg = mono_object_to_string(exception, nullptr);
                char* exChars = mono_string_to_utf8(exMsg);
                LOG_CORE_ERROR("Exception in Initialize: {}", exChars);
                mono_free(exChars);
            }
        }
        else
            LOG_CORE_ERROR("Initialize(BTBlackboard, Entity) method not found in class hierarchy!");
    }

    void ScriptEngine::CallBTNodeOnStart(MonoObject* nodeInstance)
    {
        if (!nodeInstance)
            return;

        MonoClass* klass = mono_object_get_class(nodeInstance);
        const char* className = mono_class_get_name(klass);
        const char* nameSpace = mono_class_get_namespace(klass);
        
        //std::string fullName = fmt::format("{}.{}", nameSpace, className);
        std::string fullName;
        if (nameSpace && strlen(nameSpace) != 0)
            fullName = fmt::format("{}.{}", nameSpace, className);
        else
            fullName = className;
        
        MonoMethod* method = nullptr;
        if (s_BTActionClasses.find(fullName) != s_BTActionClasses.end())
            method = s_BTActionClasses[fullName].OnStartMethod;
        else if (s_BTConditionClasses.find(fullName) != s_BTConditionClasses.end())
            method = s_BTConditionClasses[fullName].OnStartMethod;
        else if (s_BTDecoratorClasses.find(fullName) != s_BTDecoratorClasses.end())
            method = s_BTDecoratorClasses[fullName].OnStartMethod;
            
        if (method)
            mono_runtime_invoke(method, nodeInstance, nullptr, nullptr);
    }

    int ScriptEngine::CallBTNodeUpdate(MonoObject* nodeInstance)
    {
        if (!nodeInstance)
            return 1; // Failure

        MonoClass* klass = mono_object_get_class(nodeInstance);
        const char* className = mono_class_get_name(klass);
        const char* nameSpace = mono_class_get_namespace(klass);
        
        //std::string fullName = fmt::format("{}.{}", nameSpace, className);
        std::string fullName;
        if (nameSpace && strlen(nameSpace) != 0)
            fullName = fmt::format("{}.{}", nameSpace, className);
        else
            fullName = className;
        
        MonoMethod* method = nullptr;
        if (s_BTActionClasses.find(fullName) != s_BTActionClasses.end())
            method = s_BTActionClasses[fullName].UpdateMethod;
            
        if (method)
        {
            MonoObject* result = mono_runtime_invoke(method, nodeInstance, nullptr, nullptr);
            return *(int*)mono_object_unbox(result);
        }
        
        return 1; // Failure
    }

    void ScriptEngine::CallBTNodeOnFinished(MonoObject* nodeInstance)
    {
        if (!nodeInstance)
            return;

        MonoClass* klass = mono_object_get_class(nodeInstance);
        const char* className = mono_class_get_name(klass);
        const char* nameSpace = mono_class_get_namespace(klass);
        
        //std::string fullName = fmt::format("{}.{}", nameSpace, className);
        std::string fullName;
        if (nameSpace && strlen(nameSpace) != 0)
            fullName = fmt::format("{}.{}", nameSpace, className);
        else
            fullName = className;
        
        MonoMethod* method = nullptr;
        if (s_BTActionClasses.find(fullName) != s_BTActionClasses.end())
            method = s_BTActionClasses[fullName].OnFinishedMethod;
        else if (s_BTConditionClasses.find(fullName) != s_BTConditionClasses.end())
            method = s_BTConditionClasses[fullName].OnFinishedMethod;
        else if (s_BTDecoratorClasses.find(fullName) != s_BTDecoratorClasses.end())
            method = s_BTDecoratorClasses[fullName].OnFinishedMethod;
            
        if (method)
            mono_runtime_invoke(method, nodeInstance, nullptr, nullptr);
    }

    void ScriptEngine::CallBTNodeOnAbort(MonoObject* nodeInstance)
    {
        if (!nodeInstance)
            return;

        MonoClass* klass = mono_object_get_class(nodeInstance);
        const char* className = mono_class_get_name(klass);
        const char* nameSpace = mono_class_get_namespace(klass);
        
        //std::string fullName = fmt::format("{}.{}", nameSpace, className);
        std::string fullName;
        if (nameSpace && strlen(nameSpace) != 0)
            fullName = fmt::format("{}.{}", nameSpace, className);
        else
            fullName = className;
        
        MonoMethod* method = nullptr;
        if (s_BTActionClasses.find(fullName) != s_BTActionClasses.end())
            method = s_BTActionClasses[fullName].OnAbortMethod;
        else if (s_BTConditionClasses.find(fullName) != s_BTConditionClasses.end())
            method = s_BTConditionClasses[fullName].OnAbortMethod;
        else if (s_BTDecoratorClasses.find(fullName) != s_BTDecoratorClasses.end())
            method = s_BTDecoratorClasses[fullName].OnAbortMethod;
            
        if (method)
            mono_runtime_invoke(method, nodeInstance, nullptr, nullptr);
    }

    bool ScriptEngine::CallBTConditionCheck(MonoObject* conditionInstance)
    {
        if (!conditionInstance)
            return false;

        MonoClass* klass = mono_object_get_class(conditionInstance);
        const char* className = mono_class_get_name(klass);
        const char* nameSpace = mono_class_get_namespace(klass);
        
        //std::string fullName = fmt::format("{}.{}", nameSpace, className);
        std::string fullName;
        if (nameSpace && strlen(nameSpace) != 0)
            fullName = fmt::format("{}.{}", nameSpace, className);
        else
            fullName = className;
        
        if (s_BTConditionClasses.find(fullName) != s_BTConditionClasses.end())
        {
            MonoMethod* method = s_BTConditionClasses[fullName].CheckConditionMethod;
            if (method)
            {
                MonoObject* result = mono_runtime_invoke(method, conditionInstance, nullptr, nullptr);
                return *(bool*)mono_object_unbox(result);
            }
        }
        
        return false;
    }

    bool ScriptEngine::CallBTDecoratorCanExecute(MonoObject* decoratorInstance)
    {
        if (!decoratorInstance)
            return true;

        MonoClass* klass = mono_object_get_class(decoratorInstance);
        const char* className = mono_class_get_name(klass);
        const char* nameSpace = mono_class_get_namespace(klass);
        
        //std::string fullName = fmt::format("{}.{}", nameSpace, className);
        std::string fullName;
        if (nameSpace && strlen(nameSpace) != 0)
            fullName = fmt::format("{}.{}", nameSpace, className);
        else
            fullName = className;
        
        if (s_BTDecoratorClasses.find(fullName) != s_BTDecoratorClasses.end())
        {
            MonoMethod* method = s_BTDecoratorClasses[fullName].CanExecuteMethod;
            if (method)
            {
                MonoObject* result = mono_runtime_invoke(method, decoratorInstance, nullptr, nullptr);
                return *(bool*)mono_object_unbox(result);
            }
        }
        
        return true;
    }

    void ScriptEngine::CallBTDecoratorOnFinishedResult(MonoObject* decoratorInstance, NodeStatus& status)
    {
        if (!decoratorInstance)
            return;

        MonoClass* klass = mono_object_get_class(decoratorInstance);
        const char* className = mono_class_get_name(klass);
        const char* nameSpace = mono_class_get_namespace(klass);
        
        //std::string fullName = fmt::format("{}.{}", nameSpace, className);
        std::string fullName;
        if (nameSpace && strlen(nameSpace) != 0)
            fullName = fmt::format("{}.{}", nameSpace, className);
        else
            fullName = className;
        
        if (s_BTDecoratorClasses.find(fullName) != s_BTDecoratorClasses.end())
        {
            MonoMethod* method = s_BTDecoratorClasses[fullName].OnFinishedResultMethod;
            if (method)
            {
                int statusInt = (int)status;
                void* params[1] = { &statusInt };
                mono_runtime_invoke(method, decoratorInstance, params, nullptr);
                status = (NodeStatus)statusInt;
            }
        }
    }
    
    ScriptEngine::BTParameterInfo ScriptEngine::GetBTParameterInfo(const std::string& nodeClassName)
    {
        if (s_BTParameterCache.find(nodeClassName) != s_BTParameterCache.end())
            return s_BTParameterCache[nodeClassName];

        BTParameterInfo info;
        
        MonoClass* nodeClass = nullptr;
        if (s_BTActionClasses.find(nodeClassName) != s_BTActionClasses.end())
            nodeClass = s_BTActionClasses[nodeClassName].MonoClass;
        else if (s_BTConditionClasses.find(nodeClassName) != s_BTConditionClasses.end())
            nodeClass = s_BTConditionClasses[nodeClassName].MonoClass;
        else if (s_BTDecoratorClasses.find(nodeClassName) != s_BTDecoratorClasses.end())
            nodeClass = s_BTDecoratorClasses[nodeClassName].MonoClass;

        if (!nodeClass)
        {
            LOG_CORE_ERROR("Node class not found: {}", nodeClassName);
            return info;
        }
        
        MonoObject* nodeInstance = InstantiateClass(nodeClass);
        if (!nodeInstance)
        {
            LOG_CORE_ERROR("Failed to instantiate node class: {}", nodeClassName);
            return info;
        }
        
        //MonoClassField* parametersField = mono_class_get_field_from_name(nodeClass, "parameters");
        MonoClassField* parametersField = FindFieldInHierarchy(nodeClass, "parameters");
        
        /*MonoObject* paramsInstance = nullptr;
        if (parametersField)
        {
            mono_field_get_value(nodeInstance, parametersField, &paramsInstance);
        }*/
        MonoObject* paramsInstance = nullptr;
        if (parametersField)
        {
            mono_field_get_value(nodeInstance, parametersField, &paramsInstance);
            if (!paramsInstance)
            {
                LOG_CORE_ERROR("Failed to get parameters instance from field for {}", nodeClassName);
            }
        }
        
        if (!paramsInstance)
        {
            LOG_CORE_ERROR("Parameters field is null for {}", nodeClassName);
            return info;
        }

        MonoClass* paramsClass = mono_object_get_class(paramsInstance);
        info.MonoClass = paramsClass;
        //info.ClassName = mono_class_get_name(paramsClass);
        const char* paramsNs = mono_class_get_namespace(paramsClass);
        const char* paramsName = mono_class_get_name(paramsClass);
        if (paramsNs && strlen(paramsNs) > 0)
            info.ClassName = fmt::format("{}.{}", paramsNs, paramsName);
        else
            info.ClassName = paramsName;
        
        //MonoMethod* getFieldInfosMethod = mono_class_get_method_from_name(paramsClass, "GetFieldInfos", 0);
        MonoMethod* getFieldInfosMethod = FindMethodInHierarchy(paramsClass, "GetFieldInfos", 0);
        if (!getFieldInfosMethod)
        {
            LOG_CORE_ERROR("GetFieldInfos method not found on {}", info.ClassName);
            return info;
        }

        MonoArray* fieldInfoArray = (MonoArray*)mono_runtime_invoke(getFieldInfosMethod, paramsInstance, nullptr, nullptr);
        
        if (fieldInfoArray)
        {
            int length = static_cast<int>(mono_array_length(fieldInfoArray));
            LOG_CORE_INFO("Found {} parameter fields for {}", length, nodeClassName);
            
            for (int i = 0; i < length; i++)
            {
                MonoObject* fieldInfoObj = mono_array_get(fieldInfoArray, MonoObject*, i);
                MonoClass* fieldInfoClass = mono_object_get_class(fieldInfoObj);
                
                BTParameterField paramField;
                
                MonoClassField* nameField = mono_class_get_field_from_name(fieldInfoClass, "Name");
                if (nameField)
                {
                    MonoString* nameStr;
                    mono_field_get_value(fieldInfoObj, nameField, &nameStr);
                    char* name = mono_string_to_utf8(nameStr);
                    paramField.Name = name ? name : "";
                    if (name) mono_free(name);
                }
                
                MonoClassField* displayNameField = mono_class_get_field_from_name(fieldInfoClass, "DisplayName");
                if (displayNameField)
                {
                    MonoString* displayNameStr;
                    mono_field_get_value(fieldInfoObj, displayNameField, &displayNameStr);
                    char* displayName = mono_string_to_utf8(displayNameStr);
                    paramField.DisplayName = displayName ? displayName : "";
                    if (displayName) mono_free(displayName);
                }
                
                MonoClassField* isKeyField = mono_class_get_field_from_name(fieldInfoClass, "IsBlackboardKey");
                if (isKeyField)
                    mono_field_get_value(fieldInfoObj, isKeyField, &paramField.IsBlackboardKey);
                
                MonoClassField* keyTypeField = mono_class_get_field_from_name(fieldInfoClass, "BlackboardKeyType");
                if (keyTypeField)
                    mono_field_get_value(fieldInfoObj, keyTypeField, &paramField.BlackboardKeyType);
                
                paramField.Field = mono_class_get_field_from_name(paramsClass, paramField.Name.c_str());
                
                if (paramField.Field)
                {
                    MonoType* fieldType = mono_field_get_type(paramField.Field);
                    paramField.Type = MonoTypeToScriptFieldType(fieldType);
                    info.Fields.push_back(paramField);
                    
                    LOG_CORE_INFO("  Field: {} ({}) - IsKey: {}", paramField.DisplayName, ScriptFieldTypeToString(paramField.Type), paramField.IsBlackboardKey);
                }
            }
        }

        s_BTParameterCache[nodeClassName] = info;
        LOG_CORE_INFO("Cached parameter info for {} with {} fields", nodeClassName, info.Fields.size());
        return info;
    }

    MonoObject* ScriptEngine::CreateBTParameterInstance(const std::string& nodeClassName)
    {
        BTParameterInfo info = GetBTParameterInfo(nodeClassName);
    
        if (!info.MonoClass)
        {
            LOG_CORE_ERROR("Failed to get parameter info for {}", nodeClassName);
            return nullptr;
        }

        MonoObject* paramsInstance = InstantiateClass(info.MonoClass);
        if (!paramsInstance)
        {
            LOG_CORE_ERROR("Failed to instantiate parameter class for {}", nodeClassName);
            return nullptr;
        }

        LOG_CORE_INFO("Created parameter instance for {}", nodeClassName);
        return paramsInstance;
    }

    void ScriptEngine::DrawBTParametersImGui(MonoObject* paramsInstance, HBlackboard* blackboard)
    {
        if (!paramsInstance)
            return;

        MonoClass* paramsClass = mono_object_get_class(paramsInstance);
        const char* paramsClassName = mono_class_get_name(paramsClass);
        const char* paramsNamespace = mono_class_get_namespace(paramsClass);
        
        //std::string fullParamsName = fmt::format("{}.{}", paramsNamespace, paramsClassName);
        std::string fullParamsName;
        if (paramsNamespace && strlen(paramsNamespace) > 0)
            fullParamsName = fmt::format("{}.{}", paramsNamespace, paramsClassName);
        else
            fullParamsName = paramsClassName;
        
        BTParameterInfo info;
        bool found = false;
        
        for (const auto& [nodeName, cachedInfo] : s_BTParameterCache)
        
            if (cachedInfo.ClassName == fullParamsName)
            {
                info = cachedInfo;
                found = true;
                break;
            }

        
        if (!found)
            return;

        for (auto& field : info.Fields)
        {
            std::string label = field.DisplayName.empty() ? field.Name : field.DisplayName;

            if (field.IsBlackboardKey)
            {
                MonoString* keyValueStr;
                mono_field_get_value(paramsInstance, field.Field, &keyValueStr);
                
                char* keyValue = keyValueStr ? mono_string_to_utf8(keyValueStr) : nullptr;
                std::string currentKey = keyValue ? keyValue : "";
                if (keyValue)
                    mono_free(keyValue);

                const char* preview = currentKey.empty() ? "Select key..." : currentKey.c_str();

                if (ImGui::BeginCombo(label.c_str(), preview))
                {
                    if (blackboard)
                    {
                        switch (field.BlackboardKeyType)
                        {
                            case 0: // Float
                            {
                                for (const auto& [key, value] : blackboard->GetFloatValues())
                                {
                                    bool isSelected = (currentKey == key);
                                    if (ImGui::Selectable(key.c_str(), isSelected))
                                    {
                                        MonoString* monoKey = mono_string_new(s_Data->AppDomain, key.c_str());
                                        mono_field_set_value(paramsInstance, field.Field, monoKey);
                                    }
                                    if (isSelected)
                                        ImGui::SetItemDefaultFocus();
                                }
                                break;
                            }
                            case 1: // Int
                            {
                                for (const auto& [key, value] : blackboard->GetIntValues())
                                {
                                    bool isSelected = (currentKey == key);
                                    if (ImGui::Selectable(key.c_str(), isSelected))
                                    {
                                        MonoString* monoKey = mono_string_new(s_Data->AppDomain, key.c_str());
                                        mono_field_set_value(paramsInstance, field.Field, monoKey);
                                    }
                                    if (isSelected)
                                        ImGui::SetItemDefaultFocus();
                                }
                                break;
                            }
                            case 2: // Bool
                            {
                                for (const auto& [key, value] : blackboard->GetBoolValues())
                                {
                                    bool isSelected = (currentKey == key);
                                    if (ImGui::Selectable(key.c_str(), isSelected))
                                    {
                                        MonoString* monoKey = mono_string_new(s_Data->AppDomain, key.c_str());
                                        mono_field_set_value(paramsInstance, field.Field, monoKey);
                                    }
                                    if (isSelected)
                                        ImGui::SetItemDefaultFocus();
                                }
                                break;
                            }
                            case 3: // String
                            {
                                for (const auto& [key, value] : blackboard->GetStringValues())
                                {
                                    bool isSelected = (currentKey == key);
                                    if (ImGui::Selectable(key.c_str(), isSelected))
                                    {
                                        MonoString* monoKey = mono_string_new(s_Data->AppDomain, key.c_str());
                                        mono_field_set_value(paramsInstance, field.Field, monoKey);
                                    }
                                    if (isSelected)
                                        ImGui::SetItemDefaultFocus();
                                }
                                break;
                            }
                        }
                    }
                    ImGui::EndCombo();
                }
            }
            else
            {
                switch (field.Type)
                {
                    case ScriptFieldType::Float:
                    {
                        float value;
                        mono_field_get_value(paramsInstance, field.Field, &value);
                        if (ImGui::InputFloat(label.c_str(), &value))
                            mono_field_set_value(paramsInstance, field.Field, &value);
                        break;
                    }
                    case ScriptFieldType::Int:
                    {
                        int value;
                        mono_field_get_value(paramsInstance, field.Field, &value);
                        if (ImGui::InputInt(label.c_str(), &value))
                            mono_field_set_value(paramsInstance, field.Field, &value);
                        break;
                    }
                    case ScriptFieldType::Bool:
                    {
                        bool value;
                        mono_field_get_value(paramsInstance, field.Field, &value);
                        if (ImGui::Checkbox(label.c_str(), &value))
                            mono_field_set_value(paramsInstance, field.Field, &value);
                        break;
                    }
                    case ScriptFieldType::String:
                    {
                        MonoString* monoStr;
                        mono_field_get_value(paramsInstance, field.Field, &monoStr);
                        char* str = monoStr ? mono_string_to_utf8(monoStr) : nullptr;
                        
                        char buffer[256];
                        strncpy_s(buffer, str ? str : "", sizeof(buffer));
                        
                        if (ImGui::InputText(label.c_str(), buffer, sizeof(buffer)))
                        {
                            MonoString* newStr = mono_string_new(s_Data->AppDomain, buffer);
                            mono_field_set_value(paramsInstance, field.Field, newStr);
                        }
                        
                        if (str)
                            mono_free(str);
                        break;
                    }
                }
            }
        }
    }

    void ScriptEngine::SerializeBTParameters(MonoObject* paramsInstance, YAML::Emitter& out)
    {
        if (!paramsInstance)
            return;

        MonoClass* paramsClass = mono_object_get_class(paramsInstance);
        const char* paramsClassName = mono_class_get_name(paramsClass);
        const char* paramsNamespace = mono_class_get_namespace(paramsClass);
        
        std::string fullParamsName;
        if (paramsNamespace && strlen(paramsNamespace) != 0)
            fullParamsName = fmt::format("{}.{}", paramsNamespace, paramsClassName);
        else
            fullParamsName = paramsClassName;
        
        BTParameterInfo info;
        bool found = false;
        
        for (const auto& [nodeName, cachedInfo] : s_BTParameterCache)
            /*if (cachedInfo.ClassName == paramsClassName)*/if (cachedInfo.ClassName == fullParamsName)
            {
                info = cachedInfo;
                found = true;
                break;
            }

        if (!found)
            return;

        for (auto& field : info.Fields)
        {
            switch (field.Type)
            {
                case ScriptFieldType::Float:
                {
                    float value;
                    mono_field_get_value(paramsInstance, field.Field, &value);
                    out << YAML::Key << field.Name << YAML::Value << value;
                    break;
                }
                case ScriptFieldType::Int:
                {
                    int value;
                    mono_field_get_value(paramsInstance, field.Field, &value);
                    out << YAML::Key << field.Name << YAML::Value << value;
                    break;
                }
                case ScriptFieldType::Bool:
                {
                    bool value;
                    mono_field_get_value(paramsInstance, field.Field, &value);
                    out << YAML::Key << field.Name << YAML::Value << value;
                    break;
                }
                case ScriptFieldType::String:
                {
                    MonoString* monoStr;
                    mono_field_get_value(paramsInstance, field.Field, &monoStr);
                    char* str = monoStr ? mono_string_to_utf8(monoStr) : nullptr;
                    out << YAML::Key << field.Name << YAML::Value << (str ? str : "");
                    if (str)
                        mono_free(str);
                    break;
                }
            }
        }
    }

    void ScriptEngine::DeserializeBTParameters(MonoObject* paramsInstance, const YAML::Node& node)
    {
        if (!paramsInstance)
            return;

        MonoClass* paramsClass = mono_object_get_class(paramsInstance);
        const char* paramsClassName = mono_class_get_name(paramsClass);
        const char* paramsNamespace = mono_class_get_namespace(paramsClass);
        
        std::string fullParamsName;
        if (paramsNamespace && strlen(paramsNamespace) != 0)
            fullParamsName = fmt::format("{}.{}", paramsNamespace, paramsClassName);
        else
            fullParamsName = paramsClassName;
        
        BTParameterInfo info;
        bool found = false;
        
        for (const auto& [nodeName, cachedInfo] : s_BTParameterCache)
            /*if (cachedInfo.ClassName == paramsClassName)*/if (cachedInfo.ClassName == fullParamsName)
            {
                info = cachedInfo;
                found = true;
                break;
            }

        if (!found)
            return;

        for (auto& field : info.Fields)
        {
            if (!node[field.Name])
                continue;

            switch (field.Type)
            {
                case ScriptFieldType::Float:
                {
                    float value = node[field.Name].as<float>();
                    mono_field_set_value(paramsInstance, field.Field, &value);
                    break;
                }
                case ScriptFieldType::Int:
                {
                    int value = node[field.Name].as<int>();
                    mono_field_set_value(paramsInstance, field.Field, &value);
                    break;
                }
                case ScriptFieldType::Bool:
                {
                    bool value = node[field.Name].as<bool>();
                    mono_field_set_value(paramsInstance, field.Field, &value);
                    break;
                }
                case ScriptFieldType::String:
                {
                    std::string value = node[field.Name].as<std::string>();
                    MonoString* monoStr = mono_string_new(s_Data->AppDomain, value.c_str());
                    mono_field_set_value(paramsInstance, field.Field, monoStr);
                    break;
                }
            }
        }
    }

    std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetBTActionClasses()
    {
        std::unordered_map<std::string, Ref<ScriptClass>> result;
        for (const auto& [className, info] : s_BTActionClasses)
        {
            // Convert to ScriptClass format if needed
            // For now, return empty map or implement conversion
        }
        return result;
    }

    std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetBTConditionClasses()
    {
        std::unordered_map<std::string, Ref<ScriptClass>> result;
        return result;
    }

    std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetBTDecoratorClasses()
    {
        std::unordered_map<std::string, Ref<ScriptClass>> result;
        return result;
    }

    std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetBTBlackboardClasses()
    {
        std::unordered_map<std::string, Ref<ScriptClass>> result;
        return result;
    }
}
