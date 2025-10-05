
#include "HRpch.h"
#include "Scene.h"
#include "HRealEngine/Core/Components.h"
#include "ScriptableEntity.h"
#include "HRealEngine/Core/Entity.h"
#include "HRealEngine/Renderer/Renderer2D.h"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_circle_shape.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include "HRealEngine/Scripting/ScriptEngine.h"

namespace HRealEngine
{
    static b2BodyType RigidBody2DTypeToBox2D(Rigidbody2DComponent::BodyType type)
    {
        switch (type)
        {
            case Rigidbody2DComponent::BodyType::Static:   return b2_staticBody;
            case Rigidbody2DComponent::BodyType::Dynamic:  return b2_dynamicBody;
            case Rigidbody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
        }
        HREALENGINE_CORE_DEBUGBREAK(false, "Unknown body type");
        return b2_staticBody;
    }
    
    Scene::Scene()
    {
    }
    Scene::~Scene()
    {
        delete m_PhysicsWorld;
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
    template<typename Component>
    static void CopyComponentIfExist(Entity dst, Entity src)
    {
        if (src.HasComponent<Component>())
            dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
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
            const auto& name = srcRegistry.get<TagComponent>(e).Tag;
            Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
            entityMap[uuid] = (entt::entity)newEntity;
        }

        CopyComponent<TransformComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<CameraComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<ScriptComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<SpriteRendererComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<CircleRendererComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<NativeScriptComponent>(dstRegistry, srcRegistry, entityMap);
        CopyComponent<Rigidbody2DComponent>(dstRegistry, srcRegistry, entityMap);
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
        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;
        m_EntityMap[uuid] = entity;
        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_Registry.destroy(entity);
        m_EntityMap.erase(entity.GetUUID());
    }

    void Scene::OnRuntimeStart()
    {
        m_bIsRunning = true;
        
        m_PhysicsWorld = new b2World({0.0f, -9.81f});
       OnPhysics2DStart();
       {
           ScriptEngine::OnRuntimeStart(this);
           auto view = m_Registry.view<ScriptComponent>();
           for (auto e : view)
           {
             Entity entity = {e, this};
             ScriptEngine::OnCreateEntity(entity);
           }
       }
    }

    void Scene::OnRuntimeStop()
    {
        m_bIsRunning = false;
        
        OnPhysics2DStop();
        ScriptEngine::OnRuntimeStop();
    }

    void Scene::OnSimulationStart()
    {
        OnPhysics2DStart();
    }

    void Scene::OnSimulationStop()
    {
        OnPhysics2DStop();
    }

    void Scene::OnUpdateSimulation(Timestep deltaTime, EditorCamera& camera)
    {
        const int32_t velocityIterations = 6;
        const int32_t positionIterations = 2;
        m_PhysicsWorld->Step(deltaTime, velocityIterations, positionIterations);

        auto view = m_Registry.view<Rigidbody2DComponent>();
        for (auto e : view)
        {
            Entity entity = {e, this};
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

            b2Body* body = (b2Body*)rb2d.RuntimeBody;
            const auto& position = body->GetPosition();
            transform.Position.x = position.x;
            transform.Position.y = position.y;
            transform.Rotation.z = body->GetAngle();
        }
        RenderScene(camera);
    }


    void Scene::OnUpdateEditor(Timestep deltaTime, EditorCamera& camera)
    {
        RenderScene(camera);
    }

    void Scene::OnUpdateRuntime(Timestep deltaTime)
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
        }

        {
            const int32_t velocityIterations = 6;
            const int32_t positionIterations = 2;
            m_PhysicsWorld->Step(deltaTime, velocityIterations, positionIterations);

            auto view = m_Registry.view<Rigidbody2DComponent>();
            for (auto e : view)
            {
                Entity entity = {e, this};
                auto& transform = entity.GetComponent<TransformComponent>();
                auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

                b2Body* body = (b2Body*)rb2d.RuntimeBody;
                const auto& position = body->GetPosition();
                transform.Position.x = position.x;
                transform.Position.y = position.y;
                transform.Rotation.z = body->GetAngle();
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
        
        Renderer2D::BeginScene(mainCamera->GetProjectionMatrix(), cameraTransform);
        {
            auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            for (auto entity : group)
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
        Renderer2D::EndScene();
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
        auto view = m_Registry.view<TagComponent>();
        for (auto entity : view)
        {
            const TagComponent& tagComponent = view.get<TagComponent>(entity);
            if (tagComponent.Tag == name)
                return Entity{entity, this};
        }
        return {};
    }

    void Scene::DuplicateEntity(Entity entity)
    {
        Entity newEntity = CreateEntity(entity.GetName());
        
        CopyComponentIfExist<TransformComponent>(newEntity, entity);
        CopyComponentIfExist<CameraComponent>(newEntity, entity);
        CopyComponentIfExist<SpriteRendererComponent>(newEntity, entity);
        CopyComponentIfExist<CircleRendererComponent>(newEntity, entity);
        CopyComponentIfExist<NativeScriptComponent>(newEntity, entity);
        CopyComponentIfExist<Rigidbody2DComponent>(newEntity, entity);
        CopyComponentIfExist<BoxCollider2DComponent>(newEntity, entity);
        CopyComponentIfExist<CircleCollider2DComponent>(newEntity, entity);
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

    void Scene::OnPhysics2DStart()
    {
        m_PhysicsWorld = new b2World({0.0f, -9.81f});

        auto view = m_Registry.view<Rigidbody2DComponent>();
        for (auto e : view)
        {
            Entity entity = {e, this};
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

            b2BodyDef bodyDef;
            bodyDef.type = RigidBody2DTypeToBox2D(rb2d.Type);
            bodyDef.position.Set(transform.Position.x, transform.Position.y);
            bodyDef.angle = transform.Rotation.z;

            b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
            body->SetFixedRotation(rb2d.FixedRotation);
            
            rb2d.RuntimeBody = body;

            if (entity.HasComponent<BoxCollider2DComponent>())
            {
                auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

                b2PolygonShape shape;
                shape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);

                b2FixtureDef fixtureDef;
                fixtureDef.shape = &shape;
                fixtureDef.density = bc2d.Density;
                fixtureDef.friction = bc2d.Friction;
                fixtureDef.restitution = bc2d.Restitution;
                fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
                body->CreateFixture(&fixtureDef);
            }
            if (entity.HasComponent<CircleCollider2DComponent>())
            {
                auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

                b2CircleShape shape;
                shape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
                shape.m_radius = cc2d.Radius * transform.Scale.x;

                b2FixtureDef fixtureDef;
                fixtureDef.shape = &shape;
                fixtureDef.density = cc2d.Density;
                fixtureDef.friction = cc2d.Friction;
                fixtureDef.restitution = cc2d.Restitution;
                fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;
                body->CreateFixture(&fixtureDef);
            }
        }
    }

    void Scene::OnPhysics2DStop()
    {
        delete m_PhysicsWorld;
        m_PhysicsWorld = nullptr;
    }

    void Scene::RenderScene(EditorCamera& camera)
    {
        Renderer2D::BeginScene(camera);
        {
            auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            for (auto entity : group)
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
        Renderer2D::EndScene();
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
    void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
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
