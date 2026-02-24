
#include "HRpch.h"
#include "Scene.h"
#include "HRealEngine/Core/Components.h"
#include "ScriptableEntity.h"
#include "HRealEngine/Core/Entity.h"
#include "HRealEngine/Renderer/Renderer2D.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include "BehaviorTreeThings/Core/BTSerializer.h"
#include "BehaviorTreeThings/Core/Tree.h"
#include "HRealEngine/Asset/AssetManager.h"
#include "HRealEngine/Physics/Box2DWorld.h"
#include "HRealEngine/Physics/JoltWorld.h"
#include "HRealEngine/Project/Project.h"
#include "HRealEngine/Renderer/Renderer3D.h"
#include "HRealEngine/Scripting/ScriptEngine.h"


namespace HRealEngine
{
    Scene::Scene()
    {

    }
    Scene::~Scene()
    {
    }

    template<typename Component>
    static void CopyComponent(entt::registry& dst, entt::registry& src, std::unordered_map<UUID, entt::entity>& entityMap)
    {
        auto view = src.view<Component>();
        for (auto e : view)
        {
            UUID uuid = src.get<EntityIDComponent>(e).ID;
            entt::entity dstEntt = entityMap.at(uuid);

            auto& component = src.get<Component>(e);
            dst.emplace_or_replace<Component>(dstEntt, component); 
        }
    }
    template<typename... Component>
    static void CopyComponentIfExists(Entity dst, Entity src)
    {
        ([&]()
        {
            if (src.HasComponent<Component>())
                dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
        }(), ...);
    }
    template<typename... Component>
    static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
    {
        CopyComponentIfExists<Component...>(dst, src);
    }

    static int LightTypeToGPU(LightComponent::LightType t)
    {
        switch (t)
        {
        case LightComponent::LightType::Directional:
            return 0;
        case LightComponent::LightType::Point:   
            return 1;
        case LightComponent::LightType::Spot:
            return 2;
        }
        return 0;
    }


    Ref<Scene> Scene::Copy(Ref<Scene> other)
    {
        Ref<Scene> newScene = CreateRef<Scene>();
        newScene->viewportWidth = other->viewportWidth;
        newScene->viewportHeight = other->viewportHeight;

        std::unordered_map<UUID, entt::entity> entityMap;
        auto& srcRegistry = other->m_Registry;
        auto& dstRegistry = newScene->m_Registry;

        auto idView = srcRegistry.view<EntityIDComponent>();
        for (auto e : idView)
        {
            UUID uuid = srcRegistry.get<EntityIDComponent>(e).ID;
            const auto& name = srcRegistry.get<EntityNameComponent>(e).Name;
            Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
            entityMap[uuid] = (entt::entity)newEntity;
        }

        CopyComponent<ChildrenManagerComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<TransformComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<TagComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<TextComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<LightComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<CameraComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<ScriptComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<SpriteRendererComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<MeshRendererComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<BehaviorTreeComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<AIControllerComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<PerceivableComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<CircleRendererComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<NativeScriptComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<Rigidbody2DComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<Rigidbody3DComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<BoxCollider3DComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<BoxCollider2DComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<CircleCollider2DComponent>(dstRegistry, srcRegistry, entityMap);
        
        return newScene;
    }

    Entity Scene::CreateEntity(const std::string& name)
    {
        return CreateEntityWithUUID(UUID(), name);
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
    {
        Entity entity = {m_Registry.create(),this};
        entity.AddComponent<EntityIDComponent>(uuid);
        entity.AddComponent<TransformComponent>();
        auto& tag = entity.AddComponent<EntityNameComponent>();
        tag.Name = name.empty() ? "Entity" : name;
        m_EntityMap[uuid] = entity;
        return entity;
    }

    void Scene::CreatePhysicsWorld()
    {
        if (m_b2PhysicsEnabled)
        {
            if (!m_Box2DWorld)
            {
                if (m_JoltWorld)
                    m_JoltWorld = nullptr;
                m_Box2DWorld = CreateScope<Box2DWorld>(this);
            }
        }
        else
        {
            if (!m_JoltWorld)
            {
                if (m_Box2DWorld)
                    m_Box2DWorld = nullptr;
                m_JoltWorld = CreateScope<JoltWorld>(this);
            }
        }
    }

    void Scene::DestroyEntity(Entity entity)
    {
        if (entity.HasComponent<ChildrenManagerComponent>())
        {
            auto childrenCopy = entity.GetComponent<ChildrenManagerComponent>().Children;
            for (UUID childUUID : childrenCopy)
            {
                Entity child = GetEntityByUUID(childUUID);
                if (child)
                    DestroyEntity(child);
            }
        }

        RemoveParent(entity);
        
        if (m_b2PhysicsEnabled)
            m_Box2DWorld->DestroyEntityPhysics(entity);
        else 
            m_JoltWorld->DestroyEntityPhysics(entity);
        
        if (entity.HasComponent<ScriptComponent>())
        {
            auto& scriptComponent = entity.GetComponent<ScriptComponent>();
            if (ScriptEngine::IsEntityClassExist(scriptComponent.ClassName))
                ScriptEngine::OnDestroyEntity(entity);
        }
        m_EntityMap.erase(entity.GetUUID());
        m_Registry.destroy(entity);
    }

    void Scene::ReportNoiseEvent(const NoiseEvent& event)
    {
        if (m_JoltWorld)
            m_JoltWorld->ReportNoise(event);
    }

    void Scene::OnRuntimeStart()
    {
        m_bIsRunning = true;
        
        OnPhysicsStart();
        ScriptEngine::OnRuntimeStart(this);
        auto view = m_Registry.view<ScriptComponent>();
        for (auto e : view)
        {
            Entity entity = {e, this};
            ScriptEngine::OnCreateEntity(entity);
        }
        StartBTs();
    }

    void Scene::OnRuntimeStop()
    {
        m_bIsRunning = false;
        
        OnPhysicsStop();
        ScriptEngine::OnRuntimeStop();
        StopBTs();
    }

    void Scene::OnSimulationStart()
    {
        OnPhysicsStart();
        StartBTs();
    }

    void Scene::OnSimulationStop()
    {
        OnPhysicsStop();
        StopBTs();
    }

    void Scene::StartBTs()
    {
        Root::RootClear();
        auto view = m_Registry.view<BehaviorTreeComponent>();
        for (auto e : view)
        {
            Entity entity = {e, this};
            auto& btComponent = entity.GetComponent<BehaviorTreeComponent>();
            if (btComponent.BehaviorTreeAsset)
            {
                if (m_BehaviorTreeCache.find(btComponent.BehaviorTreeAsset) == m_BehaviorTreeCache.end())
                {
                    auto metaData = Project::GetActive()->GetEditorAssetManager()->GetAssetMetadata(btComponent.BehaviorTreeAsset);
                    auto path = Project::GetAssetDirectory() / metaData.FilePath;
                    auto name = metaData.FilePath.stem().string();
                    
                    YAML::Node data = YAML::LoadFile(path.string());
                    
                    BehaviorTree* bt = Root::CreateBehaviorTree(name, path.string());
                    BTSerializer serializer(bt);
                    serializer.Deserialize(data);
                    m_BehaviorTreeCache[btComponent.BehaviorTreeAsset] = data;

                    UUID ownerUUID = entity.GetUUID();
                    m_BTOwnerUUIDs[btComponent.BehaviorTreeAsset] = ownerUUID;
                    bt->SetOwner<UUID>(&m_BTOwnerUUIDs[btComponent.BehaviorTreeAsset]);
                    bt->StartTree();
                }
                else
                {
                    YAML::Node& data = m_BehaviorTreeCache.at(btComponent.BehaviorTreeAsset);
                    auto metaData = Project::GetActive()->GetEditorAssetManager()->GetAssetMetadata(btComponent.BehaviorTreeAsset);
                    auto path = Project::GetAssetDirectory() / metaData.FilePath;
                    auto name = metaData.FilePath.stem().string();
                    
                    BehaviorTree* bt = Root::CreateBehaviorTree(name, path.string());
                    BTSerializer serializer(bt);
                    serializer.Deserialize(data);
  
                    UUID ownerUUID = entity.GetUUID();
                    m_BTOwnerUUIDs[btComponent.BehaviorTreeAsset] = ownerUUID;
                    bt->SetOwner<UUID>(&m_BTOwnerUUIDs[btComponent.BehaviorTreeAsset]);
                    bt->StartTree();
                }
            }
        }
    }

    void Scene::StopBTs()
    {
        Root::RootClear();
        m_BehaviorTreeCache.clear();
        m_BTOwnerUUIDs.clear();
    }

    void Scene::OnUpdateSimulation(Timestep deltaTime, EditorCamera& camera)
    {
        if (m_b2PhysicsEnabled)
            m_Box2DWorld->UpdateSimulation2D(deltaTime, m_StepFrames);
        else 
            m_JoltWorld->UpdateSimulation3D(deltaTime, m_StepFrames);
        RenderScene(camera);
        Root::RootTick();
    }


    void Scene::OnUpdateEditor(Timestep deltaTime, EditorCamera& camera)
    {
        RenderScene(camera);
    }

    void Scene::OnUpdateRuntime(Timestep deltaTime)
    {
        if (!m_bIsPaused || m_StepFrames-- > 0)
        {
            {
                auto view = m_Registry.view<ScriptComponent>();
                for (auto e : view)
                {
                    Entity entity = {e, this};
                    ScriptEngine::OnUpdateEntity(entity, deltaTime);
                }
                m_Registry.view<NativeScriptComponent>().each([&](auto entity, auto& nativeScript)
                {
                   if (!nativeScript.Instance)
                   {
                       nativeScript.Instance = nativeScript.InstantiateScript();
                       nativeScript.Instance->m_Entity = Entity{entity, this};
                       nativeScript.Instance->OnCreate();
                   }
                    nativeScript.Instance->OnUpdate(Timestep(deltaTime));
                });

                if (m_b2PhysicsEnabled)
                    m_Box2DWorld->UpdateRuntime2D();
                else
                    m_JoltWorld->UpdateRuntime3D();
            }
            {
                if (m_b2PhysicsEnabled)
                    m_Box2DWorld->Step2DWorld(deltaTime);
                else
                {
                    m_JoltWorld->Step3DWorld(deltaTime);
                    m_JoltWorld->UpdateDebugLines(deltaTime);
                }
            }
        }

        
        Camera* mainCamera = nullptr;
        glm::mat4 cameraTransform;
        {
            auto view = m_Registry.view<TransformComponent, CameraComponent>();
            for (auto entity : view)
            {
                auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
                if (camera.PrimaryCamera)
                {
                    mainCamera = &camera.Camera;
                    cameraTransform = transform.GetTransform();
                    break;
                } 
            }
        }
        
        if (!mainCamera)
            return;
        
        LightningAndShadowSetup(glm::vec3(cameraTransform[3]));
        
        Renderer3D::BeginScene(mainCamera->GetProjectionMatrix(), cameraTransform);
        {
            auto view = m_Registry.view<TransformComponent, MeshRendererComponent>();
            for (auto entity : view)
            {
                /*auto [transform, meshRenderer] = view.get<TransformComponent, MeshRendererComponent>(entity);
                Renderer3D::DrawMesh(transform.GetTransform(), meshRenderer, (int)entity);*/
                Entity e{entity, this};
                auto& meshRenderer = e.GetComponent<MeshRendererComponent>();
                glm::mat4 worldTransform = GetWorldTransform(e);
                Renderer3D::DrawMesh(worldTransform, meshRenderer, (int)entity);
            }
        }
        Renderer3D::EndScene();
        
        Renderer2D::BeginScene(mainCamera->GetProjectionMatrix(), cameraTransform);
        {
            auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            RecalculateRenderListSprite();
            for (auto entity : m_RenderList)
            {
                auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
            }
        }
        {
            auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
            for (auto entity : view)
            {
                auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);
                Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)entity);
            }
        }
        {
            auto view = m_Registry.view<TransformComponent, TextComponent>();
            for (auto entity : view)
            {
                auto [transform, text] = view.get<TransformComponent, TextComponent>(entity);
        
                if (!text.TextString.empty())
                    Renderer2D::DrawString(text.TextString, transform.GetTransform(), text, (int)entity);
            }
        }
        Renderer2D::EndScene();

        Root::RootTick();
    }

    void Scene::OnViewportResize(uint32_t width, uint32_t height)
    {
        if (viewportWidth == width && viewportHeight == height)
            return;
        
        viewportWidth = width;
        viewportHeight = height;
        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view)
        {
            auto& cameraComponent = view.get<CameraComponent>(entity);
            if (!cameraComponent.FixedAspectRatio)
            {
                cameraComponent.Camera.SetViewportSize(width, height);
            }
        }   
    }

    Entity Scene::GetEntityByUUID(UUID uuid)
    {
        if (m_EntityMap.find(uuid) != m_EntityMap.end())
            return Entity{m_EntityMap.at(uuid), this};
        return {};
    }

    Entity Scene::GetPrimaryCameraEntity()
    {
        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view)
        {
            const auto& camera = view.get<CameraComponent>(entity);
            if (camera.PrimaryCamera)
                return Entity{entity, this};
        }
        return {};
    }

    Entity Scene::FindEntityByName(std::string_view name)
    {
        auto view = m_Registry.view<EntityNameComponent>();
        for (auto entity : view)
        {
            const EntityNameComponent& nameComponent = view.get<EntityNameComponent>(entity);
            if (nameComponent.Name == name)
                return Entity{entity, this};
        }
        return {};
    }

    void Scene::DuplicateEntity(Entity entity)
    {
        Entity newEntity = CreateEntity(entity.GetName());

        CopyComponentIfExists(AllComponents{}, newEntity, entity);
        /*CopyComponentIfExist<TransformComponent>(newEntity, entity);
        CopyComponentIfExist<CameraComponent>(newEntity, entity);
        CopyComponentIfExist<SpriteRendererComponent>(newEntity, entity);
        CopyComponentIfExist<CircleRendererComponent>(newEntity, entity);
        CopyComponentIfExist<NativeScriptComponent>(newEntity, entity);
        CopyComponentIfExist<Rigidbody2DComponent>(newEntity, entity);
        CopyComponentIfExist<BoxCollider2DComponent>(newEntity, entity);
        CopyComponentIfExist<CircleCollider2DComponent>(newEntity, entity);*/
    }

    bool Scene::DecomposeTransform(const glm::mat4& transform, glm::vec3& outPosition, glm::vec3& rotation, glm::vec3& scale)
    {
        glm::mat4 LocalMatrix(transform);

        if (glm::epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), glm::epsilon<float>()))//abs(a - b) < e
            return false;

        if (glm::epsilonNotEqual(LocalMatrix[0][3], static_cast<float>(0), glm::epsilon<float>()) ||
            glm::epsilonNotEqual(LocalMatrix[1][3], static_cast<float>(0), glm::epsilon<float>()) ||
            glm::epsilonNotEqual(LocalMatrix[2][3], static_cast<float>(0), glm::epsilon<float>()))
        {
            LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<float>(0);
            LocalMatrix[3][3] = static_cast<float>(1);
        }

        outPosition = glm::vec3(LocalMatrix[3]);
        LocalMatrix[3] = glm::vec4(0, 0, 0, LocalMatrix[3].w);

        glm::vec3 Row[3];

        for (glm::length_t i = 0; i < 3; ++i)
            for (glm::length_t j = 0; j < 3; ++j)
                Row[i][j] = LocalMatrix[i][j];

        scale.x = length(Row[0]);
        Row[0] = glm::detail::scale(Row[0], static_cast<float>(1));
        scale.y = length(Row[1]);
        Row[1] = glm::detail::scale(Row[1], static_cast<float>(1));
        scale.z = length(Row[2]);
        Row[2] = glm::detail::scale(Row[2], static_cast<float>(1));

        rotation.y = asin(-Row[0][2]);
        if (cos(rotation.y) != 0)
        {
            rotation.x = atan2(Row[1][2], Row[2][2]);
            rotation.z = atan2(Row[0][1], Row[0][0]);
        }else
        {
            rotation.x = atan2(-Row[2][0], Row[1][1]);
            rotation.z = 0;
        }
        return true;
    }

    void Scene::SetParent(Entity child, Entity parent)
    {
        if (child == parent)
            return;
        
        if (IsAncestorOf(child, parent))
            return;
        
        RemoveParent(child);

        if (!child.HasComponent<ChildrenManagerComponent>())
            child.AddComponent<ChildrenManagerComponent>();
        auto& childRel = child.GetComponent<ChildrenManagerComponent>();
        childRel.ParentHandle = parent.GetUUID();

        if (!parent.HasComponent<ChildrenManagerComponent>())
            parent.AddComponent<ChildrenManagerComponent>();
        auto& parentRel = parent.GetComponent<ChildrenManagerComponent>();
        parentRel.Children.push_back(child.GetUUID());
    }

    void Scene::RemoveParent(Entity child)
    {
        if (!child.HasComponent<ChildrenManagerComponent>())
            return;

        auto& childRel = child.GetComponent<ChildrenManagerComponent>();
        if (childRel.ParentHandle == 0)
            return;

        Entity oldParent = GetEntityByUUID(childRel.ParentHandle);
        if (oldParent)
        {
            auto& parentRel = oldParent.GetComponent<ChildrenManagerComponent>();
            auto& children = parentRel.Children;
            children.erase(std::remove(children.begin(), children.end(), child.GetUUID()), children.end());
        }

        childRel.ParentHandle = 0;
    }

    Entity Scene::GetParent(Entity entity)
    {
        if (!entity.HasComponent<ChildrenManagerComponent>())
            return {};
        UUID parentUUID = entity.GetComponent<ChildrenManagerComponent>().ParentHandle;
        if (parentUUID == 0)
            return {};
        return GetEntityByUUID(parentUUID);
    }

    std::vector<Entity> Scene::GetChildren(Entity entity)
    {
        std::vector<Entity> result;
        if (!entity.HasComponent<ChildrenManagerComponent>())
            return result;
        
        auto& rel = entity.GetComponent<ChildrenManagerComponent>();
        result.reserve(rel.Children.size());
        for (UUID childUUID : rel.Children)
        {
            Entity child = GetEntityByUUID(childUUID);
            if (child)
                result.push_back(child);
        }
        return result;
    }

    bool Scene::IsAncestorOf(Entity ancestor, Entity entity)
    {
        Entity current = GetParent(entity);
        while (current)
        {
            if (current == ancestor)
                return true;
            current = GetParent(current);
        }
        return false;
    }

    glm::mat4 Scene::GetWorldTransform(Entity entity)
    {
        glm::mat4 transform = entity.GetComponent<TransformComponent>().GetTransform();
    
        Entity parent = GetParent(entity);
        if (parent)
            transform = GetWorldTransform(parent) * transform;
    
        return transform;
    }

    void Scene::OnPhysicsStart()
    {
        if (m_Box2DWorld)
            m_Box2DWorld->Init();
        else if (m_JoltWorld)
            m_JoltWorld->Init();
    }

    void Scene::OnPhysicsStop()
    {
        if (m_b2PhysicsEnabled)
            m_Box2DWorld = nullptr;
        else
            m_JoltWorld = nullptr;
        /*delete m_PhysicsWorld2D;
        m_PhysicsWorld2D = nullptr;*/
    }

    void Scene::RenderScene(EditorCamera& camera)
    {
        LightningAndShadowSetup(camera.GetPosition());
        Renderer3D::BeginScene(camera);
        {
            auto view = m_Registry.view<TransformComponent, MeshRendererComponent>();
            for (auto entity : view)
            {
                /*auto [transform, meshRenderer] = view.get<TransformComponent, MeshRendererComponent>(entity);
                Renderer3D::DrawMesh(transform.GetTransform(), meshRenderer, (int)entity);*/
                Entity e{entity, this};
                auto& meshRenderer = e.GetComponent<MeshRendererComponent>();
                glm::mat4 worldTransform = GetWorldTransform(e);
                Renderer3D::DrawMesh(worldTransform, meshRenderer, (int)entity);
            }
        }
        Renderer3D::EndScene();
        
        Renderer2D::BeginScene(camera);
        {
            auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            RecalculateRenderListSprite();
            for (auto entity : m_RenderList)
            {
                auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                //Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);
                Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
            }
        }
        {
            auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
            for (auto entity : view)
            {
                auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);
                Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)entity);
            }
        }

        {
            auto view = m_Registry.view<TransformComponent, TextComponent>();
            for (auto entity : view)
            {
                auto [transform, text] = view.get<TransformComponent, TextComponent>(entity);
                Renderer2D::DrawString(text.TextString, transform.GetTransform(), text, (int)entity);
            }
        }
        Renderer2D::EndScene();
    }

    void Scene::LightningAndShadowSetup(const glm::vec3& cameraPosition)
    {
        Renderer3D::SetViewPosition(cameraPosition);
        std::vector<Renderer3D::LightGPU> lights;
        lights.reserve(16);

        // Directional shadow
        bool doDirShadows = false;
        glm::vec3 dirShadowDir(0.0f, -1.0f, 0.0f);

        std::vector<std::tuple<int, glm::vec3, float>> pointShadowCasters; // (gpuLightIndex, pos, farPlane)
        pointShadowCasters.reserve(16);
        
        auto lightView = m_Registry.view<TransformComponent, LightComponent>();
        for (auto e : lightView)
        {
            auto [tc, lc] = lightView.get<TransformComponent, LightComponent>(e);
            
            if (!/*doShadows*/doDirShadows && lc.Type == LightComponent::LightType::Directional && lc.CastShadows)
            {
                if (glm::length(lc.Direction) > 0.0001f)
                {
                    dirShadowDir/*shadowDir*/ = lc.Direction;
                    /*doShadows*/doDirShadows = true;
                }
            }
            if ((int)lights.size() >= 16)
                continue;

            Renderer3D::LightGPU gpu{};
            gpu.Type = LightTypeToGPU(lc.Type);
            gpu.Position = tc.Position;
            gpu.Direction = lc.Direction;
            gpu.Color = lc.Color;
            gpu.Intensity = lc.Intensity;
            gpu.Radius = lc.Radius;
            gpu.CastShadows = lc.CastShadows ? 1 : 0;

            lights.push_back(gpu);
            
            const int gpuIndex = (int)lights.size() - 1;

            if (lc.Type == LightComponent::LightType::Point && lc.CastShadows)
            {
                float farPlane = (lc.Radius > 0.01f) ? lc.Radius : 25.0f;
                pointShadowCasters.emplace_back(gpuIndex, tc.Position, farPlane);
            }
        }
        Renderer3D::SetLights(lights);
        
        if (doDirShadows/*doShadows*/)
        {
            Renderer3D::BeginShadowPass(/*shadowDir*/dirShadowDir, cameraPosition);
            auto viewShadow = m_Registry.view<TransformComponent, MeshRendererComponent>();
            for (auto entity : viewShadow)
            {
                auto [transform, meshRenderer] = viewShadow.get<TransformComponent, MeshRendererComponent>(entity);
                Renderer3D::DrawMeshShadow(transform.GetTransform(), meshRenderer);
            }
            Renderer3D::EndShadowPass();
        }
        
        if (!pointShadowCasters.empty())
        {
            Renderer3D::BeginPointShadowAtlas();

            auto viewShadow = m_Registry.view<TransformComponent, MeshRendererComponent>();

            for (uint32_t casterIndex = 0; casterIndex < pointShadowCasters.size() && casterIndex < 8; casterIndex++)
            {
                auto [lightIndex, pos, farPlane] = pointShadowCasters[casterIndex];

                Renderer3D::BeginPointShadowCaster(casterIndex, lightIndex, pos, farPlane);

                for (auto entity : viewShadow)
                {
                    auto [transform, meshRenderer] = viewShadow.get<TransformComponent, MeshRendererComponent>(entity);
                    Renderer3D::DrawMeshPointShadow(transform.GetTransform(), meshRenderer);
                }

                Renderer3D::EndPointShadowCaster();
            }

            Renderer3D::EndPointShadowAtlas();
        }
    }

    void Scene::RecalculateRenderListSprite()
    {
        m_RenderList.clear();
        
        auto view = m_Registry.view<SpriteRendererComponent>();
        for (auto entity : view)
            m_RenderList.push_back(entity);

        std::sort(m_RenderList.begin(), m_RenderList.end(),
            [&](entt::entity a, entt::entity b)
            {
                auto& sa = view.get<SpriteRendererComponent>(a);
                auto& sb = view.get<SpriteRendererComponent>(b);
                return sa.OrderInLayer > sb.OrderInLayer;
            });
    }

    template <typename T>
    void Scene::OnComponentAdded(Entity entity, T& component)
    {
        static_assert(sizeof(T) == 0);
    }
    template<>
    void Scene::OnComponentAdded<EntityIDComponent>(Entity entity, EntityIDComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<ChildrenManagerComponent>(Entity entity, ChildrenManagerComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<LightComponent>(Entity entity, LightComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<TextComponent>(Entity entity, TextComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
    {
        if (viewportWidth > 0 && viewportHeight > 0)
            component.Camera.SetViewportSize(viewportWidth, viewportHeight);
    }
    template<>
    void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<EntityNameComponent>(Entity entity, EntityNameComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<MeshRendererComponent>(Entity entity, MeshRendererComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<BehaviorTreeComponent>(Entity entity, BehaviorTreeComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<AIControllerComponent>(Entity entity, AIControllerComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<PerceivableComponent>(Entity entity, PerceivableComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<ScriptableEntity>(Entity entity, ScriptableEntity& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<Rigidbody3DComponent>(Entity entity, Rigidbody3DComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<BoxCollider3DComponent>(Entity entity, BoxCollider3DComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component)
    {
    }
}
