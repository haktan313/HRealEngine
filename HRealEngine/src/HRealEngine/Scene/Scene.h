

#pragma once
#include <entt.hpp>

#include "BehaviorTreeThings/Core/Nodes.h"
#include "HRealEngine/Asset/Asset.h"
#include "HRealEngine/Camera/EditorCamera.h"
#include "HRealEngine/Core/Timestep.h"

class BehaviorTree;

namespace HRealEngine
{
    class JoltWorld;
    class Entity;
    class Box2DWorld;
    
    class Scene : public Asset
    {
    public:
        Scene();
        ~Scene();

        static Ref<Scene> Copy(Ref<Scene> other);

        Entity CreateEntity(const std::string& name = std::string());
        Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
        void CreatePhysicsWorld();
        void DestroyEntity(Entity entity);
        entt::registry& GetRegistry() { return m_Registry; }
        
        virtual AssetType GetType() const override { return AssetType::Scene; }

        void OnRuntimeStart();
        void OnRuntimeStop();

        void OnSimulationStart();
        void OnSimulationStop();

        void StartBTs();
        void StopBTs();
        
        void OnUpdateEditor(Timestep deltaTime, EditorCamera& camera);
        void OnUpdateRuntime(Timestep deltaTime);
        void OnUpdateSimulation(Timestep deltaTime, EditorCamera& camera);
        void OnViewportResize(uint32_t width, uint32_t height);
        Entity GetEntityByUUID(UUID uuid);
        Entity GetPrimaryCameraEntity();
        Entity FindEntityByName(std::string_view name);
        bool IsRunning() const { return m_bIsRunning; }
        bool IsPaused() const { return m_bIsPaused; }
        void SetPaused(bool paused) { m_bIsPaused = paused; }
        void Step(int frames = 1) { m_StepFrames = frames; }
        void Set2DPhysicsEnabled(bool enabled) { m_b2PhysicsEnabled = enabled; }
        bool Is2DPhysicsEnabled() const { return m_b2PhysicsEnabled; }
        void DuplicateEntity(Entity entity);

        bool DecomposeTransform(const glm::mat4& transform, glm::vec3& outPosition, glm::vec3& rotation, glm::vec3& scale);
    private:
        template<typename T>
        void OnComponentAdded(Entity entity, T& component);
        void OnPhysicsStart();
        void OnPhysicsStop();
        void RenderScene(EditorCamera& camera);
        void LightningAndShadowSetup(const glm::vec3& cameraPosition);

        void RecalculateRenderListSprite();
        std::vector<entt::entity> m_RenderList;

        bool m_bIsRunning = false;
        bool m_bIsPaused = false;
        int m_StepFrames = 0;

        std::unordered_map<UUID, entt::entity> m_EntityMap;

        std::unordered_map<AssetHandle, YAML::Node> m_BehaviorTreeCache;
        
        entt::registry m_Registry;
        uint32_t viewportWidth = 0, viewportHeight = 0;
        
        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;

        Scope<JoltWorld> m_JoltWorld;
        Scope<Box2DWorld> m_Box2DWorld;
        bool m_b2PhysicsEnabled = false;
    };
}
