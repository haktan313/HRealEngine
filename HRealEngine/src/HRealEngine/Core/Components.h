

#pragma once
#include "HRealEngine/Camera/SceneCamera.h"
#include "glm/glm.hpp"
#include "HRealEngine/Core/UUID.h"
#include "HRealEngine/Renderer/Texture.h"

#include "glm/ext/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <filesystem>
#include <glm/gtx/quaternion.hpp>


namespace HRealEngine
{
    struct MeshGPU;

    struct EntityIDComponent
    {
        UUID ID;

        EntityIDComponent() = default;
        EntityIDComponent(const EntityIDComponent&) = default;
    };
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
            glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
            
            return glm::translate(glm::mat4(1.0f), Position) * rotation * glm::scale(glm::mat4(1.0f), Scale);
        }
    };

    struct SpriteRendererComponent
    {
        glm::vec4 Color {1.0f, 1.0f, 1.0f, 1.0f};
        //Ref<Texture2D> Texture;
        AssetHandle Texture = 0;
        float TilingFactor = 1.0f;
        int OrderInLayer = 0;

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent&) = default;
        SpriteRendererComponent(const glm::vec4& color) : Color(color) {}

        operator glm::vec4& () { return Color; }
        operator const glm::vec4& () const { return Color; }
    };

    struct MeshRendererComponent
    { 
        glm::vec4 Color {1.0f, 1.0f, 1.0f, 1.0f};
        //Ref<Texture2D> Texture;
        AssetHandle Texture = 0;
        //Ref<MeshGPU> Mesh;
        AssetHandle Mesh = 0;
        std::filesystem::path MeshAssetPath;
        std::vector<std::string> MaterialOverrides;
        float TilingFactor = 1.0f;
        
        MeshRendererComponent() = default;
        MeshRendererComponent(const MeshRendererComponent&) = default;
        MeshRendererComponent(const glm::vec4& color) : Color(color) {}

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
    };

    struct ScriptComponent
    {
        std::string ClassName;

        ScriptComponent() = default;
        ScriptComponent(const ScriptComponent&) = default;
    };

    class ScriptableEntity;
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
    struct Rigidbody3DComponent
    {
        enum class BodyType { Static = 0, Dynamic, Kinematic };
        enum class CollisionShape { Box = 0, Sphere, Capsule, Cylinder, Plane, Triangle };
        BodyType Type = BodyType::Static;
        CollisionShape Shape = CollisionShape::Box;
        bool FixedRotation = false;

        // Runtime
        void* RuntimeBody = nullptr;

        Rigidbody3DComponent() = default;
        Rigidbody3DComponent(const Rigidbody3DComponent&) = default;
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

    struct CircleRendererComponent
    {
        glm::vec4 Color {1.0f, 1.0f, 1.0f, 1.0f};
        float Thickness = 1.0f;
        float Fade = 0.005f;
        int OrderInLayer = 0;

        CircleRendererComponent() = default;
        CircleRendererComponent(const CircleRendererComponent&) = default;
    };

    struct CircleCollider2DComponent
    {
        glm::vec2 Offset = {0.0f, 0.0f};
        float Radius = 0.5f;

        float Density = 1.0f;
        float Friction = 0.5f;
        float Restitution = 0.0f;
        float RestitutionThreshold = 0.5f;

        // Runtime
        void* RuntimeFixture = nullptr;

        CircleCollider2DComponent() = default;
        CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
    };

    template<typename... Component>
    struct ComponentGroup
    {};
    using AllComponents = ComponentGroup<
        TransformComponent,
        TagComponent,
        CameraComponent,
        SpriteRendererComponent,
        MeshRendererComponent,
        CircleRendererComponent,
        NativeScriptComponent,
        ScriptComponent,
        Rigidbody2DComponent,
        Rigidbody3DComponent,
        BoxCollider2DComponent,
        CircleCollider2DComponent>;
}
