
//Entity.h
#pragma once
#include <entt.hpp>

#include "Components.h"
#include "Scene.h"
#include "HRealEngine/Core/UUID.h"

namespace HRealEngine
{
    class Entity
    {
    public:
        Entity() = default;
        Entity(const Entity& other) = default;
        Entity(entt::entity handle, Scene* scene);

        template <typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            T& component = sceneRef->GetRegistry().emplace<T>(entityHandle, std::forward<Args>(args)...);
            sceneRef->OnComponentAdded<T>(*this, component);
            return component;
        }
        template <typename T, typename... Args>
        T& AddOrReplaceComponent(Args&&... args)
        {
            T& component = sceneRef->GetRegistry().emplace_or_replace<T>(entityHandle, std::forward<Args>(args)...);
            sceneRef->OnComponentAdded<T>(*this, component);
            return component;
        }
        template <typename T>
        T& GetComponent()
        {
            return sceneRef->GetRegistry().get<T>(entityHandle);
        }
        template<typename T>
        bool HasComponent() const
        {
            return sceneRef->GetRegistry().all_of<T>(entityHandle);
        }
        template<typename T>
        void RemoveComponent()
        {
            sceneRef->GetRegistry().remove<T>(entityHandle);
        }

        UUID GetUUID() { return GetComponent<EntityIDComponent>().ID; }
        const std::string& GetName() { return GetComponent<TagComponent>().Tag; }
        
        operator bool() const { return entityHandle != entt::null; }
        operator entt::entity() const { return entityHandle; }
        operator uint32_t() const { return (uint32_t)entityHandle; }
        bool operator==(const Entity& other) const
        {
            return entityHandle == other.entityHandle && sceneRef == other.sceneRef;
        }
        bool operator!=(const Entity& other) const
        {
            return !(*this == other);
        }
    private:
        entt::entity entityHandle{entt::null};
        Scene* sceneRef = nullptr;
    };
}
