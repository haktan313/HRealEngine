

#pragma once
#include "Components.h"
#include "HRealEngine/Core/UUID.h"
#include <entt.hpp>
#include "HRealEngine/Scene/Scene.h"

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
            T& component = m_Scene->GetRegistry().emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<T>(*this, component);
            return component;
        }
        template <typename T, typename... Args>
        T& AddOrReplaceComponent(Args&&... args)
        {
            T& component = m_Scene->GetRegistry().emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<T>(*this, component);
            return component;
        }
        template <typename T>
        T& GetComponent()
        {
            return m_Scene->GetRegistry().get<T>(m_EntityHandle);
        }
        template<typename T>
        bool HasComponent() const
        {
            return m_Scene->GetRegistry().all_of<T>(m_EntityHandle);
        }
        template<typename T>
        void RemoveComponent()
        {
            m_Scene->GetRegistry().remove<T>(m_EntityHandle);
        }

        UUID GetUUID() { return GetComponent<EntityIDComponent>().ID; }
        const std::string& GetName() { return GetComponent<TagComponent>().Tag; }
        
        operator bool() const { return m_EntityHandle != entt::null; }
        operator entt::entity() const { return m_EntityHandle; }
        operator uint32_t() const { return (uint32_t)m_EntityHandle; }
        bool operator==(const Entity& other) const
        {
            return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
        }
        bool operator!=(const Entity& other) const
        {
            return !(*this == other);
        }
    private:
        entt::entity m_EntityHandle{entt::null};
        Scene* m_Scene = nullptr;
    };
}
