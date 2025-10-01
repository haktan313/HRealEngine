#include "HRpch.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"

#include "HRealEngine/Core/KeyCodes.h"
#include "HRealEngine/Core/Input.h"
#include "HRealEngine/Core/Entity.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include "box2d/b2_body.h"

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

    static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        Entity entity = scene->GetEntityByUUID(entityID);

        MonoType* managedType = mono_reflection_type_get_type(componentType);
        return s_EntityHasComponentFunctions.at(managedType)(entity);
    }

    static void TransformComponent_GetPosition(UUID entityID, glm::vec3* outPosition)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        Entity entity = scene->GetEntityByUUID(entityID);
        *outPosition = entity.GetComponent<TransformComponent>().Position;
    }

    static void TransformComponent_SetPosition(UUID entityID, glm::vec3* position)
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

    static void Rigidbody2DComponent_ApplyForce(UUID entityID, glm::vec2* impulse, bool wake)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        Entity entity = scene->GetEntityByUUID(entityID);
        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = (b2Body*)rb2d.RuntimeBody;
        body->ApplyForceToCenter(b2Vec2(impulse->x, impulse->y), wake);
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
        RegisterComponent(AllComponents{});
    }

    void ScriptGlue::RegisterFunctions()
    {
        HRE_ADD_INTERNAL_CALL(PrintLog);
        HRE_ADD_INTERNAL_CALL(PrintLog_Vector);
        HRE_ADD_INTERNAL_CALL(PrintLog_VectorDot);
    }
}
