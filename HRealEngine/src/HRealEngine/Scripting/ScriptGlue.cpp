#include "HRpch.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"

#include "HRealEngine/Core/KeyCodes.h"
#include "HRealEngine/Core/Input.h"
#include "HRealEngine/Core/Entity.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include "box2d/b2_body.h"
#include "Physics/Body/Body.h"
#include "Physics/Body/BodyInterface.h"

namespace HRealEngine
{
    static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFunctions;
#define HRE_ADD_INTERNAL_CALL(Name) mono_add_internal_call("HRealEngine.InternalCalls::" #Name, Name)
    
    static void PrintLog(MonoString* string, int number)
    {
        char* cString = mono_string_to_utf8(string);
        std::string str(cString);
        mono_free(cString);
        std::cout << str << ": " << number << std::endl;
    }

    static void PrintLog_Vector(glm::vec3* parameter, glm::vec3* outResult)
    {
        *outResult = glm::normalize(*parameter);
    }

    static float PrintLog_VectorDot(glm::vec3* parameter)
    {
        return glm::dot(*parameter, *parameter);
    }

    static MonoObject* GetScriptInstance(UUID entityID)
    {
        return ScriptEngine::GetManagedInstance(entityID);
    }

    static void DestroyEntity(UUID entityID)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        Entity entity = scene->GetEntityByUUID(entityID);
        if (!entity)
            return;
        scene->DestroyEntity(entity);
    }

    static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType)
    {
        /*Scene* scene = ScriptEngine::GetSceneContext();
        Entity entity = scene->GetEntityByUUID(entityID);

        MonoType* managedType = mono_reflection_type_get_type(componentType);
        return s_EntityHasComponentFunctions.at(managedType)(entity);*/  
        Scene* scene = ScriptEngine::GetSceneContext();
        Entity entity = scene->GetEntityByUUID(entityID);
        
        MonoType* managedType = mono_reflection_type_get_type(componentType);

        auto it = s_EntityHasComponentFunctions.find(managedType);
        if (it == s_EntityHasComponentFunctions.end())
        {
            const char* tn = mono_type_get_name(managedType);
            LOG_CORE_ERROR("Entity_HasComponent: Unregistered component type: {}", tn ? tn : "<null>");
            return false;
        }
        return it->second(entity);
    }

    static uint64_t Entity_FindEntityByName(MonoString* name)
    {
        char* nameCStr = mono_string_to_utf8(name);
        Scene* scene = ScriptEngine::GetSceneContext();
        Entity entity = scene->FindEntityByName(nameCStr);
        mono_free(nameCStr);
        if (!entity)
            return 0;
        return entity.GetUUID();
    }

    static void TransformComponent_GetTranslation(UUID entityID, glm::vec3* outPosition)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        Entity entity = scene->GetEntityByUUID(entityID);
        *outPosition = entity.GetComponent<TransformComponent>().Position;
    }

    static void TransformComponent_SetTranslation(UUID entityID, glm::vec3* position)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        Entity entity = scene->GetEntityByUUID(entityID);
        entity.GetComponent<TransformComponent>().Position = *position;
    }

    static void Rigidbody2DComponent_ApplyLinearImpulse(UUID entityID, glm::vec2* impulse, glm::vec2* point, bool wake)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        Entity entity = scene->GetEntityByUUID(entityID);
        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = (b2Body*)rb2d.RuntimeBody;
        body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(point->x, point->y), wake);
    }

    static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entityID, glm::vec2* impulse, bool wake)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        Entity entity = scene->GetEntityByUUID(entityID);
        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = (b2Body*)rb2d.RuntimeBody;
        body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
    }

    static void Rigidbody3DComponent_ApplyLinearImpulseToCenter(UUID entityID, glm::vec3* impulse)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        Entity entity = scene->GetEntityByUUID(entityID);
        auto& rb3d = entity.GetComponent<Rigidbody3DComponent>();
        JPH::Body* body = (JPH::Body*)rb3d.RuntimeBody;
        JPH::BodyInterface* bodyInterface = ScriptEngine::GetBodyInterface();
        bodyInterface->AddLinearVelocity(body->GetID(), JPH::Vec3(impulse->x, impulse->y, impulse->z));
    }

    static bool Input_IsKeyDown(KeyCodes keycode)
    {
        return Input::IsKeyPressed(keycode);
    }

    template<typename... Component>
    static void RegisterComponent()
    {
        //Lamda
        ([]()
        {
            std::string name = typeid(Component).name();
            size_t pos = name.find_last_of("::");
            std::string structName = name.substr(pos + 1);
            std::string managedTypename = fmt::format("HRealEngine.{}", structName);

            MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());
            if (!managedType)
            {
                LOG_CORE_ERROR("Could not find component type {}", managedTypename);
                return;
            }
            s_EntityHasComponentFunctions[managedType] = [](Entity entity) { return entity.HasComponent<Component>(); };
        }(), ...);   
    }
    template<typename... Component>
    static void RegisterComponent(ComponentGroup<Component...>)
    {
        RegisterComponent<Component...>();
    }
    void ScriptGlue::RegisterComponents()
    {
        s_EntityHasComponentFunctions.clear();
        RegisterComponent(AllComponents{});
    }

    void ScriptGlue::RegisterFunctions()
    {
        HRE_ADD_INTERNAL_CALL(PrintLog);
        HRE_ADD_INTERNAL_CALL(PrintLog_Vector);
        HRE_ADD_INTERNAL_CALL(PrintLog_VectorDot);

        HRE_ADD_INTERNAL_CALL(GetScriptInstance);
        HRE_ADD_INTERNAL_CALL(DestroyEntity);
        HRE_ADD_INTERNAL_CALL(Entity_FindEntityByName);
        HRE_ADD_INTERNAL_CALL(Entity_HasComponent);
        HRE_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
        HRE_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
        HRE_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
        HRE_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
        HRE_ADD_INTERNAL_CALL(Rigidbody3DComponent_ApplyLinearImpulseToCenter);
        HRE_ADD_INTERNAL_CALL(Input_IsKeyDown);
    }
}
