

#pragma once
#include <entt.hpp>
#include "HRealEngine/Camera/EditorCamera.h"
#include "HRealEngine/Core/Timestep.h"
#include "HRealEngine/Core/UUID.h"

class b2World;

namespace HRealEngine
{
    class Entity;
    
    class Scene
    {
    public:
        Scene();
        ~Scene();

        static Ref<Scene> Copy(Ref<Scene> other);

        Entity CreateEntity(const std::string& name = std::string());
        Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
        void DestroyEntity(Entity entity);
        entt::registry& GetRegistry() { return m_Registry; }

        void OnRuntimeStart();
        void OnRuntimeStop();

        void OnSimulationStart();
        void OnSimulationStop();
        
        void OnUpdateEditor(Timestep deltaTime, EditorCamera& camera);
        void OnUpdateSimulation(Timestep deltaTime, EditorCamera& camera);
        void OnUpdateRuntime(Timestep deltaTime);
        void OnViewportResize(uint32_t width, uint32_t height);
        Entity GetPrimaryCameraEntity();
        void DuplicateEntity(Entity entity);

        bool DecomposeTransform(const glm::mat4& transform, glm::vec3& outPosition, glm::vec3& rotation, glm::vec3& scale);
    private:
        template<typename T>
        void OnComponentAdded(Entity entity, T& component);
        void OnPhysics2DStart();
        void OnPhysics2DStop();
        void RenderScene(EditorCamera& camera);

        b2World* m_PhysicsWorld = nullptr;
        
        entt::registry m_Registry;
        uint32_t viewportWidth = 0, viewportHeight = 0;
        
        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
    };
}
