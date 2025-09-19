
//Scene.cpp
#include "Scene.h"

#include <iostream>

#include "Components.h"
#include "Entity.h"
#include "HRealEngine/Core/Logger.h"
#include "HRealEngine/Renderer/Renderer2D.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>


namespace HRealEngine
{
    Scene::Scene()
    {
    }
    Scene::~Scene()
    {
    }
    Entity Scene::CreateEntity(const std::string& name)
    {
        Entity entity = {registry.create(),this};
        entity.AddComponent<TransformComponent>();
        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;
        return entity;
    }
    void Scene::DestroyEntity(Entity entity)
    {
        registry.destroy(entity);
    }


    void Scene::OnUpdateEditor(Timestep deltaTime, EditorCamera& camera)
    {
        Renderer2D::BeginScene(camera);
        auto group = registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
        for (auto entity : group)
        {
            auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
            //Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);
            Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
        }
        Renderer2D::EndScene();
    }

    void Scene::OnUpdateRuntime(Timestep deltaTime)
    {
        {
            registry.view<NativeScriptComponent>().each([&](auto entity, auto& nativeScript)
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
        
        Camera* mainCamera = nullptr;
        glm::mat4 cameraTransform;
        {
            auto view = registry.view<TransformComponent, CameraComponent>();
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
        auto group = registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
        for (auto entity : group)
        {
            auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
            //Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);
            Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
        }
        Renderer2D::EndScene();
    }

    void Scene::OnViewportResize(uint32_t width, uint32_t height)
    {
        viewportWidth = width;
        viewportHeight = height;
        auto view = registry.view<CameraComponent>();
        for (auto entity : view)
        {
            auto& cameraComponent = view.get<CameraComponent>(entity);
            if (!cameraComponent.FixedAspectRatio)
            {
                cameraComponent.Camera.SetViewportSize(width, height);
            }
        }   
    }

    Entity Scene::GetPrimaryCameraEntity()
    {
        auto view = registry.view<CameraComponent>();
        for (auto entity : view)
        {
            const auto& camera = view.get<CameraComponent>(entity);
            if (camera.PrimaryCamera)
                return Entity{entity, this};
        }
        return {};
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

    template <typename T>
    void Scene::OnComponentAdded(Entity entity, T& component)
    {
        //static_assert(false, "Unsupported component type added to entity");
    }
    template<>
    void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
    {
        component.Camera.SetViewportSize(viewportWidth, viewportHeight);
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
}
