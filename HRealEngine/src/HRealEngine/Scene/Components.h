
//Components.h
#pragma once
#include <string>
#include "SceneCamera.h"
#include "ScriptableEntity.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "HRealEngine/Renderer/Texture.h"

namespace HRealEngine
{
    struct TransformComponent
    {
        glm::vec3 Position {0.0f, 0.0f, 0.0f};
        glm::vec3 Rotation {0.0f, 0.0f, 0.0f};
        glm::vec3 Scale {1.0f, 1.0f, 1.0f};

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3& transform) : Position(transform) {}

        glm::mat4 GetTransform() const
        {
            glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));/*glm::rotate(glm::mat4(1.0f), Rotation.x, {1,0,0})
            * glm::rotate(glm::mat4(1.0f), Rotation.y, {0,1,0})
            * glm::rotate(glm::mat4(1.0f), Rotation.z, {0,0,1});*/
            
            return glm::translate(glm::mat4(1.0f), Position) * rotation * glm::scale(glm::mat4(1.0f), Scale);
        }
    };

    struct SpriteRendererComponent
    {
        glm::vec4 Color {1.0f, 1.0f, 1.0f, 1.0f};
        Ref<Texture2D> Texture;
        float TilingFactor = 1.0f;

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent&) = default;
        SpriteRendererComponent(const glm::vec4& color) : Color(color) {}

        operator glm::vec4& () { return Color; }
        operator const glm::vec4& () const { return Color; }
    };

    struct TagComponent
    {
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string& tag) : Tag(tag) {}
    };

    struct CameraComponent
    {
        SceneCamera Camera;
        bool PrimaryCamera = true;
        bool FixedAspectRatio = false;

        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;
        //CameraComponent(const glm::mat4& projection, bool bPrimaryCamera = false) : Camera(projection), PrimaryCamera(bPrimaryCamera) {}
    };

    struct NativeScriptComponent
    {
        ScriptableEntity* Instance = nullptr;

        ScriptableEntity*(*InstantiateScript)();
        void(*DestroyScript)(NativeScriptComponent*);
        
        template<typename T>
        void Bind()
        {
            InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
            DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
        }
    };

    struct Rigidbody2DComponent
    {
        enum class BodyType { Static = 0, Dynamic, Kinematic };
        BodyType Type = BodyType::Static;
        bool FixedRotation = false;

        // Runtime
        void* RuntimeBody = nullptr;

        Rigidbody2DComponent() = default;
        Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
    };

    struct BoxCollider2DComponent
    {
        glm::vec2 Offset = {0.0f, 0.0f};
        glm::vec2 Size = {0.5f, 0.5f};

        float Density = 1.0f;
        float Friction = 0.5f;
        float Restitution = 0.0f;
        float RestitutionThreshold = 0.5f;

        // Runtime
        void* RuntimeFixture = nullptr;

        BoxCollider2DComponent() = default;
        BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
    };
}
