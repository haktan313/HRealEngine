

#pragma once
#include "Components.h"
#include <entt.hpp>
#include "HRealEngine/Core/UUID.h"
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
            auto& registry = m_Scene->GetRegistry();
            if (!registry.valid(m_EntityHandle))
                return false;
            return m_Scene->GetRegistry().all_of<T>(m_EntityHandle);
        }
        template<typename T>
        void RemoveComponent()
        {
            m_Scene->GetRegistry().remove<T>(m_EntityHandle);
        }

        UUID GetUUID()
        {
            if (!HasComponent<EntityIDComponent>())
                return 0;
            return GetComponent<EntityIDComponent>().ID;
        }
        const std::string& GetName() { return GetComponent<EntityNameComponent>().Name; }
        void Destroy() { m_Scene->DestroyEntity(*this); }
        bool HasTag(const std::string& tag) 
        {
            if (!HasComponent<TagComponent>())
                return false;
            const auto& tags = GetComponent<TagComponent>().Tags;
            return std::find(tags.begin(), tags.end(), tag) != tags.end();
        }
        void AddTag(const std::string& tag)
        {
            if (!HasComponent<TagComponent>())
                AddComponent<TagComponent>();
            auto& tags = GetComponent<TagComponent>().Tags;
            if (std::find(tags.begin(), tags.end(), tag) == tags.end())
                tags.push_back(tag);
        }
        void RemoveTag(const std::string& tag)
        {
            if (!HasComponent<TagComponent>())                
                return;
            auto& tags = GetComponent<TagComponent>().Tags;
            tags.erase(std::remove(tags.begin(), tags.end(), tag), tags.end());
        }
        
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
