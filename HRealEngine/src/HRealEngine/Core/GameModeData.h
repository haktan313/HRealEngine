#pragma once
#include <unordered_map>
#include <string>
#include <glm/glm.hpp>

namespace HRealEngine
{
    using EntityID = uint64_t;
    class GameModeData
    {
    public:
        GameModeData() = default;
        ~GameModeData() = default;
        
        void SetString(const std::string& key, const std::string& value) { m_StringData[key] = value; }
        void SetInt(const std::string& key, int value) { m_IntData[key] = value; }
        void SetFloat(const std::string& key, float value) { m_FloatData[key] = value; }
        void SetBool(const std::string& key, bool value) { m_BoolData[key] = value; }
        void SetVec2(const std::string& key, const glm::vec2& value) { m_Vec2Data[key] = value; }
        void SetVec3(const std::string& key, const glm::vec3& value) { m_Vec3Data[key] = value; }
        void SetVec4(const std::string& key, const glm::vec4& value) { m_Vec4Data[key] = value; }
        void SetEntity(const std::string& key, EntityID entityID) { m_EntityData[key] = entityID; }

        bool HasString(const std::string& key) const { return m_StringData.find(key) != m_StringData.end(); }
        bool HasInt(const std::string& key) const { return m_IntData.find(key) != m_IntData.end(); }
        bool HasFloat(const std::string& key) const { return m_FloatData.find(key) != m_FloatData.end(); }
        bool HasBool(const std::string& key) const { return m_BoolData.find(key) != m_BoolData.end(); }
        bool HasVec2(const std::string& key) const { return m_Vec2Data.find(key) != m_Vec2Data.end(); }
        bool HasVec3(const std::string& key) const { return m_Vec3Data.find(key) != m_Vec3Data.end(); }
        bool HasVec4(const std::string& key) const { return m_Vec4Data.find(key) != m_Vec4Data.end(); }
        bool HasEntity(const std::string& key) const { return m_EntityData.find(key) != m_EntityData.end(); }

        std::string GetString(const std::string& key, const std::string& defaultValue = "") const
        {
            auto it = m_StringData.find(key);
            return (it != m_StringData.end()) ? it->second : defaultValue;
        }
        int GetInt(const std::string& key, int defaultValue = 0) const
        {
            auto it = m_IntData.find(key);
            return (it != m_IntData.end()) ? it->second : defaultValue;
        }
        float GetFloat(const std::string& key, float defaultValue = 0.0f) const
        {
            auto it = m_FloatData.find(key);
            return (it != m_FloatData.end()) ? it->second : defaultValue;
        }
        bool GetBool(const std::string& key, bool defaultValue = false) const
        {
            auto it = m_BoolData.find(key);
            return (it != m_BoolData.end()) ? it->second : defaultValue;
        }
        glm::vec2 GetVec2(const std::string& key, const glm::vec2& defaultValue = glm::vec2(0.0f)) const
        {
            auto it = m_Vec2Data.find(key);
            return (it != m_Vec2Data.end()) ? it->second : defaultValue;
        }
        glm::vec3 GetVec3(const std::string& key, const glm::vec3& defaultValue = glm::vec3(0.0f)) const
        {
            auto it = m_Vec3Data.find(key);
            return (it != m_Vec3Data.end()) ? it->second : defaultValue;
        }
        glm::vec4 GetVec4(const std::string& key, const glm::vec4& defaultValue = glm::vec4(0.0f)) const
        {
            auto it = m_Vec4Data.find(key);
            return (it != m_Vec4Data.end()) ? it->second : defaultValue;
        }
        EntityID GetEntity(const std::string& key, EntityID defaultValue = 0) const
        {
            auto it = m_EntityData.find(key);
            return (it != m_EntityData.end()) ? it->second : defaultValue;
        }

        void RemoveData(const std::string& key)
        {
            m_StringData.erase(key);
            m_IntData.erase(key);
            m_FloatData.erase(key);
            m_BoolData.erase(key);
            m_Vec2Data.erase(key);
            m_Vec3Data.erase(key);
            m_Vec4Data.erase(key);
            m_EntityData.erase(key);
        }
        bool HasData(const std::string& key) const
        {
            return HasString(key) || HasInt(key) || HasFloat(key) || HasBool(key) || HasVec2(key) || HasVec3(key) || HasVec4(key) || HasEntity(key);
        }
        void ClearAllData() 
        {
            m_StringData.clear();
            m_IntData.clear();
            m_FloatData.clear();
            m_BoolData.clear();
            m_Vec2Data.clear();
            m_Vec3Data.clear();
            m_Vec4Data.clear();
            m_EntityData.clear();
        }
    private:
        std::unordered_map<std::string, std::string> m_StringData;
        std::unordered_map<std::string, int> m_IntData;
        std::unordered_map<std::string, float> m_FloatData;
        std::unordered_map<std::string, bool> m_BoolData;
        std::unordered_map<std::string, glm::vec2> m_Vec2Data;
        std::unordered_map<std::string, glm::vec3> m_Vec3Data;
        std::unordered_map<std::string, glm::vec4> m_Vec4Data;
        std::unordered_map<std::string, EntityID> m_EntityData;
    };
}
