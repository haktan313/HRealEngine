#include "HRpch.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"

#include "HRealEngine/Core/KeyCodes.h"
#include "HRealEngine/Core/Input.h"
#include "HRealEngine/Core/Entity.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include "box2d/b2_body.h"
#include "HRealEngine/Core/Application.h"
#include "HRealEngine/Core/MouseButtonCodes.h"
#include "HRealEngine/Physics/JoltWorld.h"
#include "HRealEngine/Project/Project.h"
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

	static uint64_t SpawnEntity(MonoString* name)
    {
    	auto nameCStr = mono_string_to_utf8(name);
		Scene* scene = ScriptEngine::GetSceneContext();
    	if (!scene)
		{
			LOG_CORE_ERROR("SpawnEntity: Scene context is null!");
			mono_free(nameCStr);
			return 0;
		}
		Entity entity = scene->CreateEntity(nameCStr);
		mono_free(nameCStr);
		if (!entity)
		{
			LOG_CORE_ERROR("SpawnEntity: Failed to create entity!");
			return 0;
		}
    	UUID entityID = entity.GetUUID();
    	LOG_CORE_INFO("Spawned entity with name '{}' and ID {}", mono_string_to_utf8(name), (uint64_t)entityID);
    	return entityID;
    }

	static std::array<std::string, 3> s_ComponentTypeNames = {
		"HRealEngine.BoxCollider3DComponent",
		"HRealEngine.MeshRendererComponent",
		"HRealEngine.Rigidbody3DComponent"
	};
	static void Entity_AddComponent(UUID entityID, MonoReflectionType* componentType)
    {
		auto entity = ScriptEngine::GetSceneContext()->GetEntityByUUID(entityID);
		if (!entity)
		{
			LOG_CORE_ERROR("Entity_AddComponent: Invalid entity ID: {}", (uint64_t)entityID);
			return;
		}
    	auto managedType = mono_reflection_type_get_type(componentType);
    	LOG_CORE_INFO("Adding component of type {} to entity {}", mono_type_get_name(managedType), (uint64_t)entityID);
    	auto it = s_EntityHasComponentFunctions.find(managedType);
		if (it != s_EntityHasComponentFunctions.end())
		{
			std::string componentName = mono_type_get_name(managedType);
			LOG_CORE_INFO("Component type {} is registered, checking if entity already has it", componentName);
			if (it->second(entity))
			{
				LOG_CORE_WARN("Entity {} already has component of type {}, skipping AddComponent", (uint64_t)entityID, componentName);
				return;
			}
			
			if (componentName == s_ComponentTypeNames[0])
			{
				entity.AddComponent<BoxCollider3DComponent>();
				Scene* scene = ScriptEngine::GetSceneContext();
				JoltWorld* joltWorld = scene->GetJoltWorld();
				if (joltWorld)
					joltWorld->CreateBodyForEntity(entity);
			}
			else if (componentName == s_ComponentTypeNames[1])
				entity.AddComponent<MeshRendererComponent>();
			else if (componentName == s_ComponentTypeNames[2])
			{
				entity.AddComponent<Rigidbody3DComponent>();
				Scene* scene = ScriptEngine::GetSceneContext();
				JoltWorld* joltWorld = scene->GetJoltWorld();
				if (joltWorld)
					joltWorld->CreateBodyForEntity(entity);
			}
			else
				LOG_CORE_ERROR("Entity_AddComponent: No matching component type found for {}", componentName);
		}
    }

	static void Entity_AddRigidbody3DComponent(UUID entityID, Rigidbody3DComponent::BodyType bodyType, bool fixedRotation, float friction, float restitution, float convexRadius)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		if (!scene)
		{
			LOG_CORE_ERROR("Entity_AddRigidbody3DComponent: Scene context is null!");
			return;
		}
		Entity entity = scene->GetEntityByUUID(entityID);
		if (!entity)
		{
			LOG_CORE_ERROR("Entity_AddRigidbody3DComponent: Invalid entity ID: {}", (uint64_t)entityID);
			return;
		}
		entity.AddComponent<Rigidbody3DComponent>(bodyType, fixedRotation, friction, restitution, convexRadius);
		JoltWorld* joltWorld = scene->GetJoltWorld();
		if (joltWorld)
			joltWorld->CreateBodyForEntity(entity);
		else
			LOG_CORE_ERROR("Entity_AddRigidbody3DComponent: Jolt physics world is null, cannot create body for entity {}", (uint64_t)entityID);
	}

	static void Entity_AddBoxCollider3DComponent(UUID entityID, bool isTrigger, glm::vec3* size, glm::vec3* offset)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		if (!scene)
		{
			LOG_CORE_ERROR("Entity_AddBoxCollider3DComponent: Scene context is null!");
			return;
		}
		Entity entity = scene->GetEntityByUUID(entityID);
		if (!entity)
		{
			LOG_CORE_ERROR("Entity_AddBoxCollider3DComponent: Invalid entity ID: {}", (uint64_t)entityID);
			return;
		}
		entity.AddComponent<BoxCollider3DComponent>(isTrigger, *offset, *size);
		JoltWorld* joltWorld = scene->GetJoltWorld();
		if (joltWorld)
			joltWorld->CreateBodyForEntity(entity);
		else
			LOG_CORE_ERROR("Entity_AddBoxCollider3DComponent: Jolt physics world is null, cannot create body for entity {}", (uint64_t)entityID);
	}

	static void Entity_AddMeshRendererComponent(UUID entityID, MonoString* meshPath)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		if (!scene)
		{
			LOG_CORE_ERROR("Entity_AddMeshRendererComponent: Scene context is null!");
			return;
		}
		Entity entity = scene->GetEntityByUUID(entityID);
		if (!entity)
		{
			LOG_CORE_ERROR("Entity_AddMeshRendererComponent: Invalid entity ID: {}", (uint64_t)entityID);
			return;
		}
		auto meshPathCStr = mono_string_to_utf8(meshPath);
		auto meshHandle = Project::GetActive()->GetEditorAssetManager()->GetHandleFromPath(meshPathCStr);
		mono_free(meshPathCStr);
		if (meshHandle == 0)
		{
			LOG_CORE_ERROR("Entity_AddMeshRendererComponent: Invalid mesh path: {}", meshPathCStr);
			entity.AddComponent<MeshRendererComponent>();
			return;
		}
		entity.AddComponent<MeshRendererComponent>(meshHandle);
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

	static uint64_t Entity_GetHoveredEntity()
    {
    	Entity* hoveredEntity = Input::GetHoveredEntity();
    	if (!hoveredEntity)
    		return 0;

    	if (!*hoveredEntity)
    		return 0;
    	
    	if (!hoveredEntity->HasComponent<EntityIDComponent>())
    		return 0;

    	return hoveredEntity->GetUUID();
    }


    static void OpenScene(MonoString* scenePath)
    {
        char* pathCStr = mono_string_to_utf8(scenePath);
        std::string pathStr(pathCStr);
        mono_free(pathCStr);
        ScriptEngine::OpenScene(pathStr);
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

    static void TransformComponent_GetRotation(UUID entityID, glm::vec3* outRotation)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        Entity entity = scene->GetEntityByUUID(entityID);
        *outRotation = entity.GetComponent<TransformComponent>().Rotation;
    }

    static void TransformComponent_SetRotation(UUID entityID, glm::vec3* rotation)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        Entity entity = scene->GetEntityByUUID(entityID);
        entity.GetComponent<TransformComponent>().Rotation = *rotation;
    }

	static MonoString* TextComponent_GetText(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HREALENGINE_CORE_DEBUGBREAK(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HREALENGINE_CORE_DEBUGBREAK(entity);
		HREALENGINE_CORE_DEBUGBREAK(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		return ScriptEngine::CreateString(tc.TextString.c_str());
	}

	std::string MonoStringToString(MonoString* string)
    {
    	char* cStr = mono_string_to_utf8(string);
    	std::string str(cStr);
    	mono_free(cStr);
    	return str;
    }
	static void TextComponent_SetText(UUID entityID, MonoString* textString)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HREALENGINE_CORE_DEBUGBREAK(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HREALENGINE_CORE_DEBUGBREAK(entity);
		HREALENGINE_CORE_DEBUGBREAK(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		tc.TextString = MonoStringToString(textString);
	}

	static void TextComponent_GetColor(UUID entityID, glm::vec4* color)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HREALENGINE_CORE_DEBUGBREAK(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HREALENGINE_CORE_DEBUGBREAK(entity);
		HREALENGINE_CORE_DEBUGBREAK(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		*color = tc.Color;
	}

	static void TextComponent_SetColor(UUID entityID, glm::vec4* color)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HREALENGINE_CORE_DEBUGBREAK(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HREALENGINE_CORE_DEBUGBREAK(entity);
		HREALENGINE_CORE_DEBUGBREAK(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		tc.Color = *color;
	}

	static float TextComponent_GetKerning(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HREALENGINE_CORE_DEBUGBREAK(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HREALENGINE_CORE_DEBUGBREAK(entity);
		HREALENGINE_CORE_DEBUGBREAK(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		return tc.Kerning;
	}

	static void TextComponent_SetKerning(UUID entityID, float kerning)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HREALENGINE_CORE_DEBUGBREAK(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HREALENGINE_CORE_DEBUGBREAK(entity);
		HREALENGINE_CORE_DEBUGBREAK(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		tc.Kerning = kerning;
	}

	static float TextComponent_GetLineSpacing(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HREALENGINE_CORE_DEBUGBREAK(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HREALENGINE_CORE_DEBUGBREAK(entity);
		HREALENGINE_CORE_DEBUGBREAK(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		return tc.LineSpacing;
	}

	static void TextComponent_SetLineSpacing(UUID entityID, float lineSpacing)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HREALENGINE_CORE_DEBUGBREAK(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HREALENGINE_CORE_DEBUGBREAK(entity);
		HREALENGINE_CORE_DEBUGBREAK(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		tc.LineSpacing = lineSpacing;
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

	static void Rigidbody3DComponent_SetLinearVelocity(UUID entityID, glm::vec3* velocity)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		auto& rb3d = entity.GetComponent<Rigidbody3DComponent>();
		JPH::Body* body = (JPH::Body*)rb3d.RuntimeBody;
		JPH::BodyInterface* bodyInterface = ScriptEngine::GetBodyInterface();
		bodyInterface->SetLinearVelocity(body->GetID(), JPH::Vec3(velocity->x, velocity->y, velocity->z));
	}

	static void Rigidbody3DComponent_GetLinearVelocity(UUID entityID, glm::vec3* outVelocity)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		auto& rb3d = entity.GetComponent<Rigidbody3DComponent>();
		JPH::Body* body = (JPH::Body*)rb3d.RuntimeBody;
		JPH::BodyInterface* bodyInterface = ScriptEngine::GetBodyInterface();
		JPH::Vec3 velocity = bodyInterface->GetLinearVelocity(body->GetID());
		*outVelocity = glm::vec3(velocity.GetX(), velocity.GetY(), velocity.GetZ());
	}
	
	static void Rigidbody3DComponent_SetRotationDegrees(UUID entityID, glm::vec3* eulerDeg)
    {
    	Scene* scene = ScriptEngine::GetSceneContext();
    	Entity entity = scene->GetEntityByUUID(entityID);
    	auto& rb3d = entity.GetComponent<Rigidbody3DComponent>();
    	JPH::Body* body = (JPH::Body*)rb3d.RuntimeBody;
    	JPH::BodyInterface* bi = ScriptEngine::GetBodyInterface();

    	glm::vec3 eulerRad = glm::radians(*eulerDeg);
    	glm::quat gq = glm::quat(eulerRad);
    	JPH::Quat q(gq.x, gq.y, gq.z, gq.w);

    	bi->SetRotation(body->GetID(), q, JPH::EActivation::Activate);
    }
	
	static void Rigidbody3DComponent_GetRotationDegrees(UUID entityID, glm::vec3* outRotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		auto& rb3d = entity.GetComponent<Rigidbody3DComponent>();
		JPH::Body* body = (JPH::Body*)rb3d.RuntimeBody;
		JPH::BodyInterface* bodyInterface = ScriptEngine::GetBodyInterface();
		JPH::Quat rotation = bodyInterface->GetRotation(body->GetID());
		glm::quat glmRotation(rotation.GetX(), rotation.GetY(), rotation.GetZ(), rotation.GetW());
		glm::vec3 eulerDegrees = glm::degrees(glm::eulerAngles(glmRotation));
		*outRotation = eulerDegrees;
	}

	static void Rigidbody3DComponent_SetBodyType(UUID entityID, int bodyType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		auto& rb3d = entity.GetComponent<Rigidbody3DComponent>();
		rb3d.Type = static_cast<Rigidbody3DComponent::BodyType>(bodyType);
		JoltWorld* joltWorld = scene->GetJoltWorld();
		if (joltWorld)
			joltWorld->SetBodyTypeForEntity(entity);
	}

	static int Rigidbody3DComponent_GetBodyType(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		auto& rb3d = entity.GetComponent<Rigidbody3DComponent>();
		return static_cast<int>(rb3d.Type);
	}

	static void MeshRendererComponent_SetMesh(UUID entityID, MonoString* meshPath)
    {
	    Scene* scene = ScriptEngine::GetSceneContext();
		if (!scene)
		{
			LOG_CORE_ERROR("MeshRendererComponent_SetMesh: Scene context is null!");
			return;
		}
		Entity entity = scene->GetEntityByUUID(entityID);
		if (!entity)
		{
			LOG_CORE_ERROR("MeshRendererComponent_SetMesh: Invalid entity ID: {}", (uint64_t)entityID);
			return;
		}
		if (!entity.HasComponent<MeshRendererComponent>())
		{
			LOG_CORE_ERROR("MeshRendererComponent_SetMesh: Entity {} does not have MeshRendererComponent!", (uint64_t)entityID);
			return;
		}
		auto& meshRenderer = entity.GetComponent<MeshRendererComponent>();
		char* meshPathCStr = mono_string_to_utf8(meshPath);
		meshRenderer.MeshAssetPath = meshPathCStr;
		mono_free(meshPathCStr);
    }

	static void BoxCollider3DComponent_SetSize(UUID entityID, glm::vec3* size)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		if (!entity)
		{
			LOG_CORE_ERROR("BoxCollider3DComponent_SetOffset: Invalid entity ID: {}", (uint64_t)entityID);
			return;
		}
		if (!entity.HasComponent<BoxCollider3DComponent>())
		{
			LOG_CORE_ERROR("BoxCollider3DComponent_SetOffset: Entity {} does not have BoxCollider3DComponent!", (uint64_t)entityID);
			return;
		}
		auto& boxCollider = entity.GetComponent<BoxCollider3DComponent>();
		boxCollider.Size = *size;
		JoltWorld* joltWorld = scene->GetJoltWorld();
		if (joltWorld) 
			joltWorld->SetBoxColliderSizeForEntity(entity, *size);
	}

	static glm::vec3 BoxCollider3DComponent_GetSize(UUID entityID)
    {
	    Scene* scene = ScriptEngine::GetSceneContext();
	    Entity entity = scene->GetEntityByUUID(entityID);
	    if (!entity)
	    {
		    LOG_CORE_ERROR("BoxCollider3DComponent_GetSize: Invalid entity ID: {}", (uint64_t)entityID);
	    	return glm::vec3(0.0f);
	    }
		if (!entity.HasComponent<BoxCollider3DComponent>())
		{
			LOG_CORE_ERROR("BoxCollider3DComponent_GetSize: Entity {} does not have BoxCollider3DComponent!", (uint64_t)entityID);
			return glm::vec3(0.0f);
		}
		auto& boxCollider = entity.GetComponent<BoxCollider3DComponent>();
		return boxCollider.Size;
    }

	static void BoxCollider3DComponent_SetOffset(UUID entityID, glm::vec3* offset)
	{
	    Scene* scene = ScriptEngine::GetSceneContext();
	    Entity entity = scene->GetEntityByUUID(entityID);
	    if (!entity)
	    {
		    LOG_CORE_ERROR("BoxCollider3DComponent_SetOffset: Invalid entity ID: {}", (uint64_t)entityID);
	    	return;
	    }
		if (!entity.HasComponent<BoxCollider3DComponent>())
		{
			LOG_CORE_ERROR("BoxCollider3DComponent_SetOffset: Entity {} does not have BoxCollider3DComponent!", (uint64_t)entityID);
			return;
		}
		auto& boxCollider = entity.GetComponent<BoxCollider3DComponent>();
		boxCollider.Offset = *offset;
		JoltWorld* joltWorld = scene->GetJoltWorld();
		if (joltWorld) 
			joltWorld->SetBoxColliderOffsetForEntity(entity, *offset);
	}

	static glm::vec3 BoxCollider3DComponent_GetOffset(UUID entityID)
	{
	    Scene* scene = ScriptEngine::GetSceneContext();
	    Entity entity = scene->GetEntityByUUID(entityID);
	    if (!entity)
	    {
		    LOG_CORE_ERROR("BoxCollider3DComponent_GetOffset: Invalid entity ID: {}", (uint64_t)entityID);
	    	return glm::vec3(0.0f);
	    }
		if (!entity.HasComponent<BoxCollider3DComponent>())
		{
			LOG_CORE_ERROR("BoxCollider3DComponent_GetOffset: Entity {} does not have BoxCollider3DComponent!", (uint64_t)entityID);
			return glm::vec3(0.0f);
		}
		auto& boxCollider = entity.GetComponent<BoxCollider3DComponent>();
		return boxCollider.Offset;
	}

	static void BoxCollider3DComponent_SetIsTrigger(UUID entityID, bool isTrigger)
    {
    	Scene* scene = ScriptEngine::GetSceneContext();
		if (!scene)
		{
			LOG_CORE_ERROR("BoxCollider3DComponent_SetIsTrigger: Scene context is null!");
			return;
		}
		Entity entity = scene->GetEntityByUUID(entityID);
		if (!entity)
		{
			LOG_CORE_ERROR("BoxCollider3DComponent_SetIsTrigger: Invalid entity ID: {}", (uint64_t)entityID);
			return;
		}
		if (!entity.HasComponent<BoxCollider3DComponent>())
		{
			LOG_CORE_ERROR("BoxCollider3DComponent_SetIsTrigger: Entity {} does not have BoxCollider3DComponent!", (uint64_t)entityID);
			return;
		}
		auto& boxCollider = entity.GetComponent<BoxCollider3DComponent>();
		boxCollider.bIsTrigger = isTrigger;
		JoltWorld* joltWorld = scene->GetJoltWorld();
		if (joltWorld)
			joltWorld->SetIsTriggerForEntity(entity, isTrigger);
    }

	static bool BoxCollider3DComponent_GetIsTrigger(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		if (!scene)
		{
			LOG_CORE_ERROR("BoxCollider3DComponent_GetIsTrigger: Scene context is null!");
			return false;
		}
		Entity entity = scene->GetEntityByUUID(entityID);
		if (!entity)
		{
			LOG_CORE_ERROR("BoxCollider3DComponent_GetIsTrigger: Invalid entity ID: {}", (uint64_t)entityID);
			return false;
		}
		if (!entity.HasComponent<BoxCollider3DComponent>())
		{
			LOG_CORE_ERROR("BoxCollider3DComponent_GetIsTrigger: Entity {} does not have BoxCollider3DComponent!", (uint64_t)entityID);
			return false;
		}
		auto& boxCollider = entity.GetComponent<BoxCollider3DComponent>();
		return boxCollider.bIsTrigger;
	}

    static bool Input_IsKeyDown(KeyCodes keycode)
    {
        return Input::IsKeyPressed(keycode);
    }

	static void Input_GetMousePosition(glm::vec2* outResult)
    {
    	*outResult = Input::GetViewportMousePos();
    }

	static void Input_SetCursorMode(CursorMode mode)
	{
		Input::SetCursorMode(mode);
	}

	static CursorMode Input_GetCursorMode()
	{
    	return Input::GetCursorMode();
	}

	static bool Input_IsMousePressed(int button)
	{
    	switch (button)
		{
			case 0:
    			return Input::IsMouseButtonPressed(HR_MOUSE_BUTTON_LEFT);
			case 1:
    			return Input::IsMouseButtonPressed(HR_MOUSE_BUTTON_RIGHT);
			case 2:
    			return Input::IsMouseButtonPressed(HR_MOUSE_BUTTON_MIDDLE);
			default:
				LOG_CORE_ERROR("Input_IsMousePressed: Invalid mouse button index: {}", button);
				return false;
		}
	}

	static void GameModeData_SetStringData(MonoString* key, MonoString* value)
	{
    	std::string keyStr = MonoStringToString(key);
		std::string valueStr = MonoStringToString(value);
		Application::Get().GetGameModeData().SetString(keyStr, valueStr);
	}
	static std::string GameModeData_GetStringData(MonoString* key)
	{
    	std::string keyStr = MonoStringToString(key);
		return Application::Get().GetGameModeData().GetString(keyStr);
	}
	static void GameModeData_SetFloatData(MonoString* key, float value)
	{
		std::string keyStr = MonoStringToString(key);
    	std::string valueStr = MonoStringToString(key);
		Application::Get().GetGameModeData().SetFloat(keyStr, value);
	}
	static float GameModeData_GetFloatData(MonoString* key)
	{
    	std::string keyStr = MonoStringToString(key);
		return Application::Get().GetGameModeData().GetFloat(keyStr);
	}
	static void GameModeData_SetIntData(MonoString* key, int value)
    {
	    std::string keyStr = MonoStringToString(key);
    	std::string valueStr = MonoStringToString(key);
    }
	static int GameModeData_GetIntData(MonoString* key)
	{
		std::string keyStr = MonoStringToString(key);
    	std::string valueStr = MonoStringToString(key);
		return Application::Get().GetGameModeData().GetInt(keyStr);
	}
	static void GameModeData_SetVector2Data(MonoString* key, glm::vec2* value)
	{
		std::string keyStr = MonoStringToString(key);
		std::string valueStr = MonoStringToString(key);
    	Application::Get().GetGameModeData().SetVec2(keyStr, *value);
	}
	static glm::vec2 GameModeData_GetVector2Data(MonoString* key)
    {
	    std::string keyStr = MonoStringToString(key);
    	std::string valueStr = MonoStringToString(key);
    	return Application::Get().GetGameModeData().GetVec2(keyStr);
    }
	static void GameModeData_SetVector3Data(MonoString* key, glm::vec3* value)
    {
	    std::string keyStr = MonoStringToString(key);
    	std::string valueStr = MonoStringToString(key);
    	Application::Get().GetGameModeData().SetVec3(keyStr, *value);
    }
	static glm::vec3 GameModeData_GetVector3Data(MonoString* key)
	{
	    std::string keyStr = MonoStringToString(key);
		std::string valueStr = MonoStringToString(key);
		return Application::Get().GetGameModeData().GetVec3(keyStr);
	}
	static void GameModeData_SetVector4Data(MonoString* key, glm::vec4* value)
	{
	    std::string keyStr = MonoStringToString(key);
		std::string valueStr = MonoStringToString(key);
		Application::Get().GetGameModeData().SetVec4(keyStr, *value);
	}
	static glm::vec4 GameModeData_GetVector4Data(MonoString* key)
	{	    std::string keyStr = MonoStringToString(key);
		std::string valueStr = MonoStringToString(key);
		return Application::Get().GetGameModeData().GetVec4(keyStr);
	}
	static void GameModeData_SetEntityData(MonoString* key, uint64_t entityID)
	{
	    std::string keyStr = MonoStringToString(key);
		Application::Get().GetGameModeData().SetEntity(keyStr, entityID);
	}
	static uint64_t GameModeData_GetEntityData(MonoString* key)
	{
    	std::string keyStr = MonoStringToString(key);
		return Application::Get().GetGameModeData().GetEntity(keyStr);
	}
	
	static bool GameModeData_HasData(MonoString* key)
	{	    std::string keyStr = MonoStringToString(key);
		return Application::Get().GetGameModeData().HasString(keyStr) || Application::Get().GetGameModeData().HasInt(keyStr) || Application::Get().GetGameModeData().HasFloat(keyStr) || Application::Get().GetGameModeData().HasBool(keyStr) || Application::Get().GetGameModeData().HasVec2(keyStr) || Application::Get().GetGameModeData().HasVec3(keyStr) || Application::Get().GetGameModeData().HasVec4(keyStr) || Application::Get().GetGameModeData().HasEntity(keyStr);
	}
	static void GameModeData_RemoveData(MonoString* key)
	{
    	std::string keyStr = MonoStringToString(key);
		Application::Get().GetGameModeData().RemoveData(keyStr);
	}
	static void GameModeData_ClearAllData()
	{
    	Application::Get().GetGameModeData().ClearAllData();
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
    	HRE_ADD_INTERNAL_CALL(SpawnEntity);
    	HRE_ADD_INTERNAL_CALL(Entity_AddComponent);
		HRE_ADD_INTERNAL_CALL(Entity_AddRigidbody3DComponent);
		HRE_ADD_INTERNAL_CALL(Entity_AddBoxCollider3DComponent);
		HRE_ADD_INTERNAL_CALL(Entity_AddMeshRendererComponent);
        HRE_ADD_INTERNAL_CALL(Entity_FindEntityByName);
    	HRE_ADD_INTERNAL_CALL(Entity_GetHoveredEntity);
        HRE_ADD_INTERNAL_CALL(OpenScene);
        HRE_ADD_INTERNAL_CALL(Entity_HasComponent);
        HRE_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
        HRE_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
        HRE_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
        HRE_ADD_INTERNAL_CALL(TransformComponent_SetRotation);

        HRE_ADD_INTERNAL_CALL(TextComponent_GetText);
        HRE_ADD_INTERNAL_CALL(TextComponent_SetText);
        HRE_ADD_INTERNAL_CALL(TextComponent_GetColor);
        HRE_ADD_INTERNAL_CALL(TextComponent_SetColor);
        HRE_ADD_INTERNAL_CALL(TextComponent_GetKerning);
        HRE_ADD_INTERNAL_CALL(TextComponent_SetKerning);
        HRE_ADD_INTERNAL_CALL(TextComponent_GetLineSpacing);
        HRE_ADD_INTERNAL_CALL(TextComponent_SetLineSpacing);
        
        HRE_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
        HRE_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
        HRE_ADD_INTERNAL_CALL(Rigidbody3DComponent_ApplyLinearImpulseToCenter);
    	HRE_ADD_INTERNAL_CALL(Rigidbody3DComponent_SetLinearVelocity);
    	HRE_ADD_INTERNAL_CALL(Rigidbody3DComponent_GetLinearVelocity);
    	HRE_ADD_INTERNAL_CALL(Rigidbody3DComponent_SetRotationDegrees);
    	HRE_ADD_INTERNAL_CALL(Rigidbody3DComponent_GetRotationDegrees);
		HRE_ADD_INTERNAL_CALL(Rigidbody3DComponent_SetBodyType);
		HRE_ADD_INTERNAL_CALL(Rigidbody3DComponent_GetBodyType);

    	HRE_ADD_INTERNAL_CALL(MeshRendererComponent_SetMesh);

    	HRE_ADD_INTERNAL_CALL(BoxCollider3DComponent_SetSize);
    	HRE_ADD_INTERNAL_CALL(BoxCollider3DComponent_GetSize);
		HRE_ADD_INTERNAL_CALL(BoxCollider3DComponent_SetOffset);
    	HRE_ADD_INTERNAL_CALL(BoxCollider3DComponent_GetOffset);
    	HRE_ADD_INTERNAL_CALL(BoxCollider3DComponent_SetIsTrigger);
		HRE_ADD_INTERNAL_CALL(BoxCollider3DComponent_GetIsTrigger);
    	
        HRE_ADD_INTERNAL_CALL(Input_IsKeyDown);
    	HRE_ADD_INTERNAL_CALL(Input_GetMousePosition);
    	HRE_ADD_INTERNAL_CALL(Input_SetCursorMode);
		HRE_ADD_INTERNAL_CALL(Input_GetCursorMode);
    	HRE_ADD_INTERNAL_CALL(Input_IsMousePressed);

    	HRE_ADD_INTERNAL_CALL(GameModeData_SetStringData);
    	HRE_ADD_INTERNAL_CALL(GameModeData_GetStringData);
		HRE_ADD_INTERNAL_CALL(GameModeData_SetFloatData);
    	HRE_ADD_INTERNAL_CALL(GameModeData_GetFloatData);
		HRE_ADD_INTERNAL_CALL(GameModeData_SetIntData);
		HRE_ADD_INTERNAL_CALL(GameModeData_GetIntData);
    	HRE_ADD_INTERNAL_CALL(GameModeData_SetVector2Data);
		HRE_ADD_INTERNAL_CALL(GameModeData_GetVector2Data);
    	HRE_ADD_INTERNAL_CALL(GameModeData_SetVector3Data);
		HRE_ADD_INTERNAL_CALL(GameModeData_GetVector3Data);
		HRE_ADD_INTERNAL_CALL(GameModeData_SetVector4Data);
    	HRE_ADD_INTERNAL_CALL(GameModeData_GetVector4Data);
		HRE_ADD_INTERNAL_CALL(GameModeData_SetEntityData);
		HRE_ADD_INTERNAL_CALL(GameModeData_GetEntityData);
    	HRE_ADD_INTERNAL_CALL(GameModeData_HasData);
		HRE_ADD_INTERNAL_CALL(GameModeData_RemoveData);
		HRE_ADD_INTERNAL_CALL(GameModeData_ClearAllData);
    }
}
