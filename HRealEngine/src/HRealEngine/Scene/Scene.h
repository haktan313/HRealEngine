
//Scene.h
#pragma once
#include <entt.hpp>
#include <string>
#include "glm/glm.hpp"
#include "HRealEngine/Core/Timestep.h"
#include "HRealEngine/Renderer/EditorCamera.h"

class b2World;

namespace HRealEngine
{
    class Entity;
    
    class Scene
    {
    public:
        Scene();
        ~Scene();

        Entity CreateEntity(const std::string& name = std::string());
        void DestroyEntity(Entity entity);
        entt::registry& GetRegistry() { return registry; }

        void OnRuntimeStart();
        void OnRuntimeStop();
        
        void OnUpdateEditor(Timestep deltaTime, EditorCamera& camera);
        void OnUpdateRuntime(Timestep deltaTime);
        void OnViewportResize(uint32_t width, uint32_t height);
        Entity GetPrimaryCameraEntity();

        bool DecomposeTransform(const glm::mat4& transform, glm::vec3& outPosition, glm::vec3& rotation, glm::vec3& scale);
    private:
        template<typename T>
        void OnComponentAdded(Entity entity, T& component);

        b2World* m_PhysicsWorld = nullptr;
        
        entt::registry registry;
        uint32_t viewportWidth = 0, viewportHeight = 0;
        
        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
    };
}
