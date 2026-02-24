#include "HRpch.h"
#include "ScriptGlue.h"

#include <mono/metadata/appdomain.h>

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
#include "HRealEngine/Utils/PlatformUtils.h"
#include "Physics/Body/Body.h"
#include "Physics/Body/BodyInterface.h"

namespace HRealEngine
{
#define HRE_ADD_INTERNAL_CALL_GLOBAL(Name) mono_add_internal_call("HRealEngine.Calls.InternalCalls_GlobalCalls::" #Name, Name)
#define HRE_ADD_INTERNAL_CALL_ENTITY(Name) mono_add_internal_call("HRealEngine.Calls.InternalCalls_Entity::" #Name, Name)
#define HRE_ADD_INTERNAL_CALL_AICONTROLLER(Name) mono_add_internal_call("HRealEngine.Calls.InternalCalls_AIController::" #Name, Name)
#define HRE_ADD_INTERNAL_CALL_PERCEIVABLE(Name) mono_add_internal_call("HRealEngine.Calls.InternalCalls_Perceivable::" #Name, Name)
#define HRE_ADD_INTERNAL_CALL_TRANSFORMCOMPONENT(Name) mono_add_internal_call("HRealEngine.Calls.InternalCalls_TransformComponent::" #Name, Name)
#define HRE_ADD_INTERNAL_CALL_RIGIDBODY(Name) mono_add_internal_call("HRealEngine.Calls.InternalCalls_Rigidbody::" #Name, Name)
#define HRE_ADD_INTERNAL_CALL_MESHRENDERER(Name) mono_add_internal_call("HRealEngine.Calls.InternalCalls_MeshRenderer::" #Name, Name)
#define HRE_ADD_INTERNAL_CALL_BOXCOLLIDER(Name) mono_add_internal_call("HRealEngine.Calls.InternalCalls_BoxCollider::" #Name, Name)
#define HRE_ADD_INTERNAL_CALL_INPUT(Name) mono_add_internal_call("HRealEngine.Calls.InternalCalls_Input::" #Name, Name)
#define HRE_ADD_INTERNAL_CALL_GAMEMODEDATA(Name) mono_add_internal_call("HRealEngine.Calls.InternalCalls_GameModeData::" #Name, Name)
#define HRE_ADD_INTERNAL_CALL_TEXTCOMPONENT(Name) mono_add_internal_call("HRealEngine.Calls.InternalCalls_TextComponent::" #Name, Name)	
    static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFunctions;
    
	static void OpenScene(MonoString* scenePath)
	{
		char* pathCStr = mono_string_to_utf8(scenePath);
		std::string pathStr(pathCStr);
		mono_free(pathCStr);
		ScriptEngine::OpenScene(pathStr);
	}

	static float Time_GetDeltaTime()
    {
	    return Time::GetDeltaTime();
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

	static uint64_t FindEntityByName(MonoString* name)
	{
		char* tagCStr = mono_string_to_utf8(name);
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->FindEntityByName(tagCStr);
		mono_free(tagCStr);
		if (!entity)
			return 0;
		return entity.GetUUID();
	}

    static MonoObject* GetScriptInstance(UUID entityID)
    {
        return ScriptEngine::GetManagedInstance(entityID);
    }

	static bool Raycast3D(glm::vec3* origin, glm::vec3* direction, float maxDistance, MonoArray* ignoreEntitiesIDs,
		uint64_t* outEntityID, glm::vec3* outPoint, glm::vec3* outNormal, float* outDistance, bool debugDraw, float debugLifetime)
	{
		std::vector<uint64_t> ignoreList;
		if (ignoreEntitiesIDs != nullptr)
		{
			uintptr_t length = mono_array_length(ignoreEntitiesIDs);
			for (uintptr_t i = 0; i < length; i++)
			{
				ignoreList.push_back(mono_array_get(ignoreEntitiesIDs, uint64_t, i));
			}
		}
		
		Scene* scene = ScriptEngine::GetSceneContext();
		if (!scene)
		{
			LOG_CORE_ERROR("Raycast3D: Scene context is null!");
			return false;
		}
		JoltWorld* joltWorld = scene->GetJoltWorld();
		if (!joltWorld)
		{
			LOG_CORE_ERROR("Raycast3D: Jolt physics world is null!");
			return false;
		}

		auto result = joltWorld->Raycast(*origin, *direction, maxDistance, debugDraw, debugLifetime, ignoreList);
		if (result.Hit)
		{
			*outEntityID = result.HitEntityID;
			*outPoint = result.HitPoint;
			*outNormal = result.HitNormal;
			*outDistance = result.Distance;
		}
		return result.Hit;
	}

	struct RaycastHitManaged
	{
		uint64_t EntityID;
		glm::vec3 Point;
		glm::vec3 Normal;
		float Distance;
	};
	static MonoArray* Raycast3DArray(glm::vec3* origin, glm::vec3* direction, MonoArray* ignoreEntitiesIDs,
		float maxDistance, bool debugDraw, float debugLifetime)
	{
		std::vector<uint64_t> ignoreList;
		if (ignoreEntitiesIDs != nullptr)
		{
			uintptr_t length = mono_array_length(ignoreEntitiesIDs);
			for (uintptr_t i = 0; i < length; i++)
			{
				ignoreList.push_back(mono_array_get(ignoreEntitiesIDs, uint64_t, i));
			}
		}
		
		Scene* scene = ScriptEngine::GetSceneContext();
		if (!scene)
		{
			LOG_CORE_ERROR("Raycast3DArray: Scene context is null!");
			return nullptr;
		}
		JoltWorld* joltWorld = scene->GetJoltWorld();
		if (!joltWorld)
		{
			LOG_CORE_ERROR("Raycast3DArray: Jolt physics world is null!");
			return nullptr;
		}

		auto results = joltWorld->RaycastAll(*origin, *direction, maxDistance, debugDraw, debugLifetime, ignoreList);
		if (results.empty())
			return nullptr;
		
		MonoClass* hitClass = mono_class_from_name(ScriptEngine::GetCoreAssemblyImage(), "HRealEngine", "RaycastHit");
    
		if (!hitClass)
		{
			LOG_CORE_ERROR("Raycast3DArray: Could not find RaycastHit class!");
			return nullptr;
		}

		MonoArray* array = mono_array_new(mono_domain_get(), hitClass, (uintptr_t)results.size());

		for (size_t i = 0; i < results.size(); i++)
		{
			RaycastHitManaged hit;
			hit.EntityID = results[i].HitEntityID;
			hit.Point = results[i].HitPoint;
			hit.Normal = results[i].HitNormal;
			hit.Distance = results[i].Distance;
			
			char* elem = mono_array_addr_with_size(array, (int)sizeof(RaycastHitManaged), (int)i);
			memcpy(elem, &hit, sizeof(RaycastHitManaged));
		}
		return array;
	}
	
	static void ReportNoiseEvent(UUID entityID, glm::vec3* position, float loudness, float maxRange, int sourceType)
	{
		auto scene = ScriptEngine::GetSceneContext();
		if (!scene)
		{
			LOG_CORE_ERROR("ReportNoiseEvent: Scene context is null!");
			return;
		}
		NoiseEvent event;
		event.SourceEntityID = entityID;
		event.Position = *position;
		event.Loudness = loudness;
		event.MaxRange = maxRange;
		event.SourceType = static_cast<PerceivableType>(sourceType);
		scene->ReportNoiseEvent(event);
	}
	

	struct PercaptionResultManaged
	{
	    uint64_t EntityID;
	    int Type;
	    int Method;
	    glm::vec3 SensedPosition;
	    float TimeSinceLastSensed;
	};
	static int AIController_GetCurrentPerceptionCount(UUID entityID)
	{
	    Scene* scene = ScriptEngine::GetSceneContext();
	    if (!scene)
	    {
		    LOG_CORE_ERROR("AIController_GetCurrentPerceptionCount: Scene context is null!");
		    return 0;
	    }
	    Entity entity = scene->GetEntityByUUID(entityID);
	    if (!entity || !entity.HasComponent<AIControllerComponent>())
	    {
	    	LOG_CORE_ERROR("AIController_GetCurrentPerceptionCount: Invalid entity ID or entity does not have AIControllerComponent! Entity ID: {}", (uint64_t)entityID);
	    	return 0;
	    }
	    return (int)entity.GetComponent<AIControllerComponent>().CurrentPerceptions.size();
	}

	static int AIController_GetForgottenPerceptionCount(UUID entityID)
	{
	    Scene* scene = ScriptEngine::GetSceneContext();
	    if (!scene)
	    {
		    LOG_CORE_ERROR("AIController_GetForgottenPerceptionCount: Scene context is null!");
	    	return 0;
	    }
	    Entity entity = scene->GetEntityByUUID(entityID);
	    if (!entity || !entity.HasComponent<AIControllerComponent>())
	    {
		    LOG_CORE_ERROR("AIController_GetForgottenPerceptionCount: Invalid entity ID or entity does not have AIControllerComponent! Entity ID: {}", (uint64_t)entityID);
	    	return 0;
	    }
	    return (int)entity.GetComponent<AIControllerComponent>().ForgottenPerceptions.size();
	}

	static MonoArray* AIController_GetCurrentPerceptions(UUID entityID)
	{
	    Scene* scene = ScriptEngine::GetSceneContext();
	    if (!scene)
	    {
		    LOG_CORE_ERROR("AIController_GetCurrentPerceptions: Scene context is null!");
		    return nullptr;
	    }
	    Entity entity = scene->GetEntityByUUID(entityID);
	    if (!entity || !entity.HasComponent<AIControllerComponent>())
	    {
		    LOG_CORE_ERROR("AIController_GetCurrentPerceptions: Invalid entity ID or entity does not have AIControllerComponent! Entity ID: {}", (uint64_t)entityID);
		    return nullptr;
	    }
	    
	    auto& ai = entity.GetComponent<AIControllerComponent>();
	    if (ai.CurrentPerceptions.empty())
	    {
		    LOG_CORE_INFO("AIController_GetCurrentPerceptions: No current perceptions for entity ID {}", (uint64_t)entityID);
		    return nullptr;
	    }
	    
	    MonoClass* resultClass = mono_class_from_name(ScriptEngine::GetCoreAssemblyImage(), "HRealEngine", "PerceptionResult");
	    if (!resultClass)
	    {
		    LOG_CORE_ERROR("AIController_GetCurrentPerceptions: Could not find PerceptionResult class!");
		    return nullptr;
	    }
	    
	    MonoArray* array = mono_array_new(mono_domain_get(), resultClass, (uintptr_t)ai.CurrentPerceptions.size());
	    for (size_t i = 0; i < ai.CurrentPerceptions.size(); i++)
	    {
	        auto& p = ai.CurrentPerceptions[i];
	        PercaptionResultManaged managed;
	        managed.EntityID = p.EntityID.ID;
	        managed.Type = (int)p.Type;
	        managed.Method = (int)p.PercaptionMethod;
	        managed.SensedPosition = p.SensedPosition;
	        managed.TimeSinceLastSensed = p.TimeSinceLastSensed;
	        
	        char* elem = mono_array_addr_with_size(array, (int)sizeof(PercaptionResultManaged), (int)i);
	        memcpy(elem, &managed, sizeof(PercaptionResultManaged));
	    }
	    return array;
	}

	static MonoArray* AIController_GetForgottenPerceptions(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		if (!scene)
		{
			LOG_CORE_ERROR("AIController_GetForgottenPerceptions: Scene context is null!");
			return nullptr;
		}
		Entity entity = scene->GetEntityByUUID(entityID);
		if (!entity || !entity.HasComponent<AIControllerComponent>())
		{
			LOG_CORE_ERROR("AIController_GetForgottenPerceptions: Invalid entity ID or entity does not have AIControllerComponent! Entity ID: {}", (uint64_t)entityID);
			return nullptr;
		}
	    
		auto& ai = entity.GetComponent<AIControllerComponent>();
		if (ai.ForgottenPerceptions.empty())
		{
			LOG_CORE_INFO("AIController_GetForgottenPerceptions: No forgotten perceptions for entity ID {}", (uint64_t)entityID);
			return nullptr;
		}
	    
		MonoClass* resultClass = mono_class_from_name(ScriptEngine::GetCoreAssemblyImage(), "HRealEngine", "PerceptionResult");
		if (!resultClass)
		{
			LOG_CORE_ERROR("AIController_GetForgottenPerceptions: Could not find PerceptionResult class!");
			return nullptr;
		}
	    
	    MonoArray* array = mono_array_new(mono_domain_get(), resultClass, (uintptr_t)ai.ForgottenPerceptions.size());
	    for (size_t i = 0; i < ai.ForgottenPerceptions.size(); i++)
	    {
	        auto& p = ai.ForgottenPerceptions[i];
	        PercaptionResultManaged managed;
	        managed.EntityID = p.EntityID.ID;
	        managed.Type = (int)p.Type;
	        managed.Method = (int)p.PercaptionMethod;
	        managed.SensedPosition = p.SensedPosition;
	        managed.TimeSinceLastSensed = p.TimeSinceLastSensed;
	        
	        char* elem = mono_array_addr_with_size(array, (int)sizeof(PercaptionResultManaged), (int)i);
	        memcpy(elem, &managed, sizeof(PercaptionResultManaged));
	    }
	    return array;
	}

	static bool AIController_IsEntityPerceived(UUID entityID, uint64_t targetEntityID)
	{
	    Scene* scene = ScriptEngine::GetSceneContext();
	    if (!scene)
	    {
		    LOG_CORE_ERROR("AIController_IsEntityPerceived: Scene context is null!");
		    return false;
	    }
	    Entity entity = scene->GetEntityByUUID(entityID);
	    if (!entity || !entity.HasComponent<AIControllerComponent>())
	    {
		    LOG_CORE_ERROR("AIController_IsEntityPerceived: Invalid entity ID or entity does not have AIControllerComponent! Entity ID: {}", (uint64_t)entityID);
		    return false;
	    }
	    
	    auto& ai = entity.GetComponent<AIControllerComponent>();
	    for (auto& p : ai.CurrentPerceptions)
	        if (p.EntityID.ID == targetEntityID)
	            return true;
	    return false;
	}

	static bool AIController_IsEntityForgotten(UUID entityID, uint64_t targetEntityID)
	{
	    Scene* scene = ScriptEngine::GetSceneContext();
	    if (!scene)
	    {
		    LOG_CORE_ERROR("AIController_IsEntityForgotten: Scene context is null!");
		    return false;
	    }
	    Entity entity = scene->GetEntityByUUID(entityID);
	    if (!entity || !entity.HasComponent<AIControllerComponent>())
	    {
		    LOG_CORE_ERROR("AIController_IsEntityForgotten: Invalid entity ID or entity does not have AIControllerComponent! Entity ID: {}", (uint64_t)entityID);
		    return false;
	    }
	    
	    auto& ai = entity.GetComponent<AIControllerComponent>();
	    for (auto& f : ai.ForgottenPerceptions)
	        if (f.EntityID.ID == targetEntityID)
	            return true;
	    return false;
	}

	static void PerceivableComponent_GetType(UUID entityID, int* outType)
	{
	    Scene* scene = ScriptEngine::GetSceneContext();
	    Entity entity = scene->GetEntityByUUID(entityID);
	    auto& pc = entity.GetComponent<PerceivableComponent>();
	    *outType = pc.Types.empty() ? 0 : (int)pc.Types[0];
	}

	static void PerceivableComponent_SetType(UUID entityID, int type)
	{
	    Scene* scene = ScriptEngine::GetSceneContext();
	    Entity entity = scene->GetEntityByUUID(entityID);
	    auto& pc = entity.GetComponent<PerceivableComponent>();
	    pc.Types.clear();
	    pc.Types.push_back((PerceivableType)type);
	}

	static bool PerceivableComponent_GetIsDetectable(UUID entityID)
	{
	    Scene* scene = ScriptEngine::GetSceneContext();
	    Entity entity = scene->GetEntityByUUID(entityID);
	    return entity.GetComponent<PerceivableComponent>().bIsDetectable;
	}

	static void PerceivableComponent_SetIsDetectable(UUID entityID, bool isDetectable)
	{
	    Scene* scene = ScriptEngine::GetSceneContext();
	    Entity entity = scene->GetEntityByUUID(entityID);
	    entity.GetComponent<PerceivableComponent>().bIsDetectable = isDetectable;
	}

	static int PerceivableComponent_GetDetectablePointCount(UUID entityID)
	{
	    Scene* scene = ScriptEngine::GetSceneContext();
	    Entity entity = scene->GetEntityByUUID(entityID);
	    return (int)entity.GetComponent<PerceivableComponent>().DetectablePointsOffsets.size();
	}

	static void PerceivableComponent_GetDetectablePoint(UUID entityID, int index, glm::vec3* outPoint)
	{
	    Scene* scene = ScriptEngine::GetSceneContext();
	    Entity entity = scene->GetEntityByUUID(entityID);
	    auto& offsets = entity.GetComponent<PerceivableComponent>().DetectablePointsOffsets;
	    if (index >= 0 && index < (int)offsets.size())
	        *outPoint = offsets[index];
	    else
	        *outPoint = glm::vec3(0.0f);
	}

	static void PerceivableComponent_SetDetectablePoint(UUID entityID, int index, glm::vec3* point)
	{
	    Scene* scene = ScriptEngine::GetSceneContext();
	    Entity entity = scene->GetEntityByUUID(entityID);
	    auto& offsets = entity.GetComponent<PerceivableComponent>().DetectablePointsOffsets;
	    if (index >= 0 && index < (int)offsets.size())
	        offsets[index] = *point;
	}

	static void PerceivableComponent_AddDetectablePoint(UUID entityID, glm::vec3* point)
	{
	    Scene* scene = ScriptEngine::GetSceneContext();
	    Entity entity = scene->GetEntityByUUID(entityID);
	    entity.GetComponent<PerceivableComponent>().DetectablePointsOffsets.push_back(*point);
	}

	static void PerceivableComponent_RemoveDetectablePoint(UUID entityID, int index)
	{
	    Scene* scene = ScriptEngine::GetSceneContext();
	    Entity entity = scene->GetEntityByUUID(entityID);
	    auto& offsets = entity.GetComponent<PerceivableComponent>().DetectablePointsOffsets;
	    if (index >= 0 && index < (int)offsets.size())
	        offsets.erase(offsets.begin() + index);
	}

	static void PerceivableComponent_ClearDetectablePoints(UUID entityID)
	{
	    Scene* scene = ScriptEngine::GetSceneContext();
	    Entity entity = scene->GetEntityByUUID(entityID);
	    entity.GetComponent<PerceivableComponent>().DetectablePointsOffsets.clear();
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

	static bool Entity_HasTag(UUID entityID, MonoString* tag)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		if (!scene)
		{
			LOG_CORE_ERROR("HasTag: Scene context is null!");
			return false;
		}
		Entity entity = scene->GetEntityByUUID(entityID);
		if (!entity)
		{
			LOG_CORE_ERROR("HasTag: Invalid entity ID: {}", (uint64_t)entityID);
			return false;
		}
		auto tagCStr = mono_string_to_utf8(tag);
		bool hasTag = entity.HasTag(tagCStr);
		mono_free(tagCStr);
		return hasTag;
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

	static std::string Entity_GetName(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		if (!entity)
			return {};
		return entity.GetName();
	}

	static void Entity_SetName(UUID entityID, MonoString* name)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		if (!entity)
			return;
		char* nameCStr = mono_string_to_utf8(name);
		entity.GetComponent<EntityNameComponent>().Name = nameCStr;
		mono_free(nameCStr);
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
	
	static uint64_t Entity_GetParent(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		if (!entity)
			return 0;
		if (!entity.HasComponent<ChildrenManagerComponent>())
			return 0;
		return entity.GetComponent<ChildrenManagerComponent>().ParentHandle;
	}

	static void Entity_SetParent(UUID childID, UUID parentID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity child = scene->GetEntityByUUID(childID);
		Entity parent = scene->GetEntityByUUID(parentID);
		if (!child || !parent)
		{
			LOG_CORE_ERROR("Entity_SetParent: Invalid entity IDs: child={}, parent={}", (uint64_t)childID, (uint64_t)parentID);
			return;
		}
		scene->SetParent(child, parent);
	}

	static void Entity_RemoveParent(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		if (!entity)
			return;
		scene->RemoveParent(entity);
	}

	static int Entity_GetChildCount(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		if (!entity || !entity.HasComponent<ChildrenManagerComponent>())
			return 0;
		return (int)entity.GetComponent<ChildrenManagerComponent>().Children.size();
	}

	static void Entity_AddChild(UUID parentID, UUID childID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity parent = scene->GetEntityByUUID(parentID);
		Entity child = scene->GetEntityByUUID(childID);
		if (!parent || !child)
		{
			LOG_CORE_ERROR("Entity_AddChild: Invalid entity IDs: parent={}, child={}", (uint64_t)parentID, (uint64_t)childID);
			return;
		}
		scene->SetParent(child, parent);
	}

	static uint64_t Entity_GetChild(UUID entityID, int index)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		if (!entity || !entity.HasComponent<ChildrenManagerComponent>())
			return 0;
		auto& children = entity.GetComponent<ChildrenManagerComponent>().Children;
		if (index < 0 || index >= (int)children.size())
			return 0;
		return children[index];
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

	static void MeshRendererComponent_SetPivotOffset(UUID entityID, glm::vec3* pivotOffset)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		if (!scene)
		{
			LOG_CORE_ERROR("MeshRendererComponent_SetPivotOffset: Scene context is null!");
			return;
		}
		Entity entity = scene->GetEntityByUUID(entityID);
		if (!entity)
		{
			LOG_CORE_ERROR("MeshRendererComponent_SetPivotOffset: Invalid entity ID: {}", (uint64_t)entityID);
			return;
		}
		if (!entity.HasComponent<MeshRendererComponent>())
		{
			LOG_CORE_ERROR("MeshRendererComponent_SetPivotOffset: Entity {} does not have MeshRendererComponent!", (uint64_t)entityID);
			return;
		}
		auto& meshRenderer = entity.GetComponent<MeshRendererComponent>();
		meshRenderer.PivotOffset = *pivotOffset;
	}

	static glm::vec3 MeshRendererComponent_GetPivotOffset(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		if (!scene)
		{
			LOG_CORE_ERROR("MeshRendererComponent_GetPivotOffset: Scene context is null!");
			return glm::vec3(0.0f);
		}
		Entity entity = scene->GetEntityByUUID(entityID);
		if (!entity)
		{
			LOG_CORE_ERROR("MeshRendererComponent_GetPivotOffset: Invalid entity ID: {}", (uint64_t)entityID);
			return glm::vec3(0.0f);
		}
		if (!entity.HasComponent<MeshRendererComponent>())
		{
			LOG_CORE_ERROR("MeshRendererComponent_GetPivotOffset: Entity {} does not have MeshRendererComponent!", (uint64_t)entityID);
			return glm::vec3(0.0f);
		}
		auto& meshRenderer = entity.GetComponent<MeshRendererComponent>();
		return meshRenderer.PivotOffset;
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

	
	std::string MonoStringToString(MonoString* string)
	{
		char* cStr = mono_string_to_utf8(string);
		std::string str(cStr);
		mono_free(cStr);
		return str;
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
        HRE_ADD_INTERNAL_CALL_GLOBAL(OpenScene);
		HRE_ADD_INTERNAL_CALL_GLOBAL(Time_GetDeltaTime);
        HRE_ADD_INTERNAL_CALL_GLOBAL(DestroyEntity);
    	HRE_ADD_INTERNAL_CALL_GLOBAL(SpawnEntity);
		HRE_ADD_INTERNAL_CALL_GLOBAL(FindEntityByName);
        HRE_ADD_INTERNAL_CALL_GLOBAL(GetScriptInstance);
		HRE_ADD_INTERNAL_CALL_GLOBAL(Raycast3D);
		HRE_ADD_INTERNAL_CALL_GLOBAL(Raycast3DArray);
		HRE_ADD_INTERNAL_CALL_GLOBAL(ReportNoiseEvent);
		
		HRE_ADD_INTERNAL_CALL_AICONTROLLER(AIController_GetCurrentPerceptionCount);
		HRE_ADD_INTERNAL_CALL_AICONTROLLER(AIController_GetForgottenPerceptionCount);
		HRE_ADD_INTERNAL_CALL_AICONTROLLER(AIController_GetCurrentPerceptions);
		HRE_ADD_INTERNAL_CALL_AICONTROLLER(AIController_GetForgottenPerceptions);
		HRE_ADD_INTERNAL_CALL_AICONTROLLER(AIController_IsEntityPerceived);
		HRE_ADD_INTERNAL_CALL_AICONTROLLER(AIController_IsEntityForgotten);
		
		HRE_ADD_INTERNAL_CALL_PERCEIVABLE(PerceivableComponent_GetType);
		HRE_ADD_INTERNAL_CALL_PERCEIVABLE(PerceivableComponent_SetType);
		HRE_ADD_INTERNAL_CALL_PERCEIVABLE(PerceivableComponent_GetIsDetectable);
		HRE_ADD_INTERNAL_CALL_PERCEIVABLE(PerceivableComponent_SetIsDetectable);
		HRE_ADD_INTERNAL_CALL_PERCEIVABLE(PerceivableComponent_GetDetectablePointCount);
		HRE_ADD_INTERNAL_CALL_PERCEIVABLE(PerceivableComponent_GetDetectablePoint);
		HRE_ADD_INTERNAL_CALL_PERCEIVABLE(PerceivableComponent_SetDetectablePoint);
		HRE_ADD_INTERNAL_CALL_PERCEIVABLE(PerceivableComponent_AddDetectablePoint);
		HRE_ADD_INTERNAL_CALL_PERCEIVABLE(PerceivableComponent_RemoveDetectablePoint);
		HRE_ADD_INTERNAL_CALL_PERCEIVABLE(PerceivableComponent_ClearDetectablePoints);

    	HRE_ADD_INTERNAL_CALL_ENTITY(Entity_AddComponent);
		HRE_ADD_INTERNAL_CALL_ENTITY(Entity_AddRigidbody3DComponent);
		HRE_ADD_INTERNAL_CALL_ENTITY(Entity_AddBoxCollider3DComponent);
		HRE_ADD_INTERNAL_CALL_ENTITY(Entity_AddMeshRendererComponent);
		HRE_ADD_INTERNAL_CALL_ENTITY(Entity_GetName);
		HRE_ADD_INTERNAL_CALL_ENTITY(Entity_SetName);
        HRE_ADD_INTERNAL_CALL_ENTITY(Entity_HasComponent);
		HRE_ADD_INTERNAL_CALL_ENTITY(Entity_HasTag);
    	HRE_ADD_INTERNAL_CALL_ENTITY(Entity_GetHoveredEntity);
		HRE_ADD_INTERNAL_CALL_ENTITY(Entity_GetParent);
		HRE_ADD_INTERNAL_CALL_ENTITY(Entity_SetParent);
		HRE_ADD_INTERNAL_CALL_ENTITY(Entity_RemoveParent);
		HRE_ADD_INTERNAL_CALL_ENTITY(Entity_GetChildCount);
		HRE_ADD_INTERNAL_CALL_ENTITY(Entity_AddChild);
		HRE_ADD_INTERNAL_CALL_ENTITY(Entity_GetChild);
		
        HRE_ADD_INTERNAL_CALL_TRANSFORMCOMPONENT(TransformComponent_GetPosition);
        HRE_ADD_INTERNAL_CALL_TRANSFORMCOMPONENT(TransformComponent_SetPosition);
        HRE_ADD_INTERNAL_CALL_TRANSFORMCOMPONENT(TransformComponent_GetRotation);
        HRE_ADD_INTERNAL_CALL_TRANSFORMCOMPONENT(TransformComponent_SetRotation);

        HRE_ADD_INTERNAL_CALL_RIGIDBODY(Rigidbody2DComponent_ApplyLinearImpulse);
        HRE_ADD_INTERNAL_CALL_RIGIDBODY(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
        HRE_ADD_INTERNAL_CALL_RIGIDBODY(Rigidbody3DComponent_ApplyLinearImpulseToCenter);
    	HRE_ADD_INTERNAL_CALL_RIGIDBODY(Rigidbody3DComponent_SetLinearVelocity);
    	HRE_ADD_INTERNAL_CALL_RIGIDBODY(Rigidbody3DComponent_GetLinearVelocity);
    	HRE_ADD_INTERNAL_CALL_RIGIDBODY(Rigidbody3DComponent_SetRotationDegrees);
    	HRE_ADD_INTERNAL_CALL_RIGIDBODY(Rigidbody3DComponent_GetRotationDegrees);
		HRE_ADD_INTERNAL_CALL_RIGIDBODY(Rigidbody3DComponent_SetBodyType);
		HRE_ADD_INTERNAL_CALL_RIGIDBODY(Rigidbody3DComponent_GetBodyType);
		
    	HRE_ADD_INTERNAL_CALL_MESHRENDERER(MeshRendererComponent_SetMesh);
		HRE_ADD_INTERNAL_CALL_MESHRENDERER(MeshRendererComponent_SetPivotOffset);
		HRE_ADD_INTERNAL_CALL_MESHRENDERER(MeshRendererComponent_GetPivotOffset);

    	HRE_ADD_INTERNAL_CALL_BOXCOLLIDER(BoxCollider3DComponent_SetSize);
    	HRE_ADD_INTERNAL_CALL_BOXCOLLIDER(BoxCollider3DComponent_GetSize);
		HRE_ADD_INTERNAL_CALL_BOXCOLLIDER(BoxCollider3DComponent_SetOffset);
    	HRE_ADD_INTERNAL_CALL_BOXCOLLIDER(BoxCollider3DComponent_GetOffset);
    	HRE_ADD_INTERNAL_CALL_BOXCOLLIDER(BoxCollider3DComponent_SetIsTrigger);
		HRE_ADD_INTERNAL_CALL_BOXCOLLIDER(BoxCollider3DComponent_GetIsTrigger);
		
        HRE_ADD_INTERNAL_CALL_INPUT(Input_IsKeyDown);
    	HRE_ADD_INTERNAL_CALL_INPUT(Input_GetMousePosition);
    	HRE_ADD_INTERNAL_CALL_INPUT(Input_SetCursorMode);
		HRE_ADD_INTERNAL_CALL_INPUT(Input_GetCursorMode);
    	HRE_ADD_INTERNAL_CALL_INPUT(Input_IsMousePressed);
		
    	HRE_ADD_INTERNAL_CALL_GAMEMODEDATA(GameModeData_SetStringData);
    	HRE_ADD_INTERNAL_CALL_GAMEMODEDATA(GameModeData_GetStringData);
		HRE_ADD_INTERNAL_CALL_GAMEMODEDATA(GameModeData_SetFloatData);
    	HRE_ADD_INTERNAL_CALL_GAMEMODEDATA(GameModeData_GetFloatData);
		HRE_ADD_INTERNAL_CALL_GAMEMODEDATA(GameModeData_SetIntData);
		HRE_ADD_INTERNAL_CALL_GAMEMODEDATA(GameModeData_GetIntData);
    	HRE_ADD_INTERNAL_CALL_GAMEMODEDATA(GameModeData_SetVector2Data);
		HRE_ADD_INTERNAL_CALL_GAMEMODEDATA(GameModeData_GetVector2Data);
    	HRE_ADD_INTERNAL_CALL_GAMEMODEDATA(GameModeData_SetVector3Data);
		HRE_ADD_INTERNAL_CALL_GAMEMODEDATA(GameModeData_GetVector3Data);
		HRE_ADD_INTERNAL_CALL_GAMEMODEDATA(GameModeData_SetVector4Data);
    	HRE_ADD_INTERNAL_CALL_GAMEMODEDATA(GameModeData_GetVector4Data);
		HRE_ADD_INTERNAL_CALL_GAMEMODEDATA(GameModeData_SetEntityData);
		HRE_ADD_INTERNAL_CALL_GAMEMODEDATA(GameModeData_GetEntityData);
    	HRE_ADD_INTERNAL_CALL_GAMEMODEDATA(GameModeData_HasData);
		HRE_ADD_INTERNAL_CALL_GAMEMODEDATA(GameModeData_RemoveData);
		HRE_ADD_INTERNAL_CALL_GAMEMODEDATA(GameModeData_ClearAllData);
		
        HRE_ADD_INTERNAL_CALL_TEXTCOMPONENT(TextComponent_GetText);
        HRE_ADD_INTERNAL_CALL_TEXTCOMPONENT(TextComponent_SetText);
        HRE_ADD_INTERNAL_CALL_TEXTCOMPONENT(TextComponent_GetColor);
        HRE_ADD_INTERNAL_CALL_TEXTCOMPONENT(TextComponent_SetColor);
        HRE_ADD_INTERNAL_CALL_TEXTCOMPONENT(TextComponent_GetKerning);
        HRE_ADD_INTERNAL_CALL_TEXTCOMPONENT(TextComponent_SetKerning);
        HRE_ADD_INTERNAL_CALL_TEXTCOMPONENT(TextComponent_GetLineSpacing);
        HRE_ADD_INTERNAL_CALL_TEXTCOMPONENT(TextComponent_SetLineSpacing);
		
    }
}
