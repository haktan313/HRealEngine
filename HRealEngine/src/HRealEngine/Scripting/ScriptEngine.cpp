#include "HRpch.h"
#include "ScriptEngine.h"

#include <fstream>
#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>

namespace HRealEngine
{
    struct ScriptEngineData
    {
        MonoDomain* RootDomain = nullptr;
        MonoDomain* AppDomain = nullptr;

        MonoAssembly* CoreAssembly = nullptr;
    };
    static ScriptEngineData* s_Data = nullptr;
    
    void ScriptEngine::Init()
    {
        s_Data = new ScriptEngineData();
        InitMono();
    }

    void ScriptEngine::Shutdown()
    {
        ShutdownMono();
        delete s_Data;
    }

    //------------------------------------------------------------------ 
    //Mono Embedding for Game Engines
    char* ReadBytes(const std::string& filepath, uint32_t* outSize)
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
    MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath)
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

        MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
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
 
    void ScriptEngine::InitMono()
    {
        mono_set_assemblies_path("mono/lib");

        MonoDomain* domain = mono_jit_init("HRealEngineJITRuntime");
        HREALENGINE_CORE_DEBUGBREAK(domain, "Failed to initialize Mono JIT Runtime!");

        s_Data->RootDomain = domain;
        s_Data->AppDomain = mono_domain_create_appdomain("HRealEngineAppDomain", nullptr);
        mono_domain_set(s_Data->AppDomain, true);

        s_Data->CoreAssembly = LoadCSharpAssembly("Resources/Scripts/HRealEngine-ScriptCore.dll");
        PrintAssemblyTypes(s_Data->CoreAssembly);

        MonoImage* coreImage = mono_assembly_get_image(s_Data->CoreAssembly);
        MonoClass* monoClass = mono_class_from_name(coreImage, "HRealEngine", "Main");
        MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
        mono_runtime_object_init(instance);

        MonoMethod* method = mono_class_get_method_from_name(monoClass, "PrintHello", 0);
        mono_runtime_invoke(method, instance, nullptr, nullptr);

        MonoMethod* printNumberMethod = mono_class_get_method_from_name(monoClass, "PrintNumber", 1);
        int value = 42;
        //void* param = (void*)42;
        void* param = &value;
        mono_runtime_invoke(printNumberMethod, instance, &param, nullptr);

        MonoMethod* printNumbersMethod = mono_class_get_method_from_name(monoClass, "PrintNumbers", 2);
        int value1 = 3;
        int value2 = 1;
        void* params2[2]
        {
            &value1, &value2
        };
        mono_runtime_invoke(printNumbersMethod, instance, params2, nullptr);

        MonoMethod* printCustomMessageMethod = mono_class_get_method_from_name(monoClass, "PrintCustomMessage", 1);
        MonoString* message = mono_string_new(s_Data->AppDomain, "Hello from C++!");
        void* stringParam = message;
        mono_runtime_invoke(printCustomMessageMethod, instance, &stringParam, nullptr);
    }

    void ScriptEngine::ShutdownMono()
    {
        //mono_domain_unload(s_Data->AppDomain);
        s_Data->AppDomain = nullptr;
        //mono_jit_cleanup(s_Data->RootDomain);
        s_Data->RootDomain = nullptr;
    }
}
