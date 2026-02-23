

#pragma once
#include "HRealEngine/Camera/SceneCamera.h"
#include "glm/glm.hpp"
#include "HRealEngine/Core/UUID.h"
#include "HRealEngine/Renderer/Texture.h"

#include "glm/ext/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <filesystem>
#include <glm/gtx/quaternion.hpp>

#include "HRealEngine/Renderer/Font.h"


namespace HRealEngine
{
    class MeshGPU;

    struct EntityIDComponent
    {
        UUID ID;

        EntityIDComponent() = default;
        EntityIDComponent(const EntityIDComponent&) = default;
    };
    struct ChildrenManagerComponent
    {
        UUID ParentHandle = 0;
        std::vector<UUID> Children{};
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
    struct LightComponent
    {
        enum class LightType { Directional = 0, Point, Spot };
        LightType Type = LightType::Point;
        glm::vec3 Color {1.0f, 1.0f, 1.0f};
        glm::vec3 Direction {0.0f, -1.0f, 0.0f};
        float Intensity = 1.0f;
        float Radius = 1.0f;
        bool CastShadows = true;

        LightComponent() = default;
        LightComponent(const LightComponent&) = default;
    };
    struct TextComponent
    {
        std::string TextString;
        Ref<Font> FontAsset = Font::GetDefault();
        glm::vec4 Color{ 1.0f };
        float Kerning = 0.0f;
        float LineSpacing = 0.0f;
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
        glm::vec3 PivotOffset {0.0f, 0.0f, 0.0f};
        glm::vec4 Color {1.0f, 1.0f, 1.0f, 1.0f};
        //Ref<Texture2D> Texture;
        AssetHandle Texture = 0;
        //Ref<MeshGPU> Mesh;
        AssetHandle Mesh = 0;
        std::filesystem::path MeshAssetPath;
        //std::vector<std::string> MaterialOverrides;
        std::vector<AssetHandle> MaterialHandleOverrides;
        float TilingFactor = 1.0f;
        
        MeshRendererComponent() = default;
        MeshRendererComponent(const MeshRendererComponent&) = default;
        MeshRendererComponent(const glm::vec4& color) : Color(color) {}
        MeshRendererComponent(AssetHandle meshHandle) : Mesh(meshHandle) {}
        MeshRendererComponent(std::filesystem::path meshPath) : MeshAssetPath(meshPath) {}

        operator glm::vec4& () { return Color; }
        operator const glm::vec4& () const { return Color; }
    };

    struct BehaviorTreeComponent
    {
        AssetHandle BehaviorTreeAsset = 0;
        
        BehaviorTreeComponent() = default;
        BehaviorTreeComponent(const BehaviorTreeComponent&) = default;
    };
    
    enum class PercaptionType { Sight = 0, Hearing, Touch };
    enum class PerceivableType { Player = 0, Enemy, Neutral, Environment };
    struct SightConfig
    {
        float SightRadius = 10.0f;
        float FieldOfView = 90.0f; // In degrees
        float ForgetDuration = 5.0f; // How long an entity remains in memory after being sensed
        
        std::vector<PerceivableType> DetectableTypes; // Types of entities that can be detected by sight
        
        SightConfig() = default;
        SightConfig(const SightConfig&) = default;
    };
    struct HearingConfig
    {
        float HearingRadius = 10.0f;
        float ForgetDuration = 5.0f; // How long an entity remains in memory after being sensed
        
        std::vector<PerceivableType> DetectableTypes; // Types of entities that can be detected by hearing
        
        HearingConfig() = default;
        HearingConfig(const HearingConfig&) = default;
    };
    struct PercaptionResult
    {
        EntityIDComponent EntityID;
        PerceivableType Type;
        PercaptionType PercaptionMethod;
        
        glm::vec3 SensedPosition;
        float TimeSinceLastSensed = 0.0f;
        
        PercaptionResult() = default;
        PercaptionResult(const PercaptionResult&) = default;
    };
    struct AIControllerComponent
    {
        std::unordered_map<PercaptionType, bool> EnabledPerceptions; // Which perception types are enabled for this AI controller
        
        SightConfig SightSettings;
        HearingConfig HearingSettings;
        
        float UpdateInterval = 0.5f; // How often the AI controller updates its perceptions and decisions
        
        std::vector<PercaptionResult> CurrentPerceptions; 
        std::vector<PercaptionResult> PreviousPerceptions;
        std::vector<PercaptionResult> ForgottenPerceptions;
        
        AIControllerComponent() = default;
        AIControllerComponent(const AIControllerComponent&) = default;
        
        bool IsSightEnabled() const 
        { 
            auto it = EnabledPerceptions.find(PercaptionType::Sight);
            return it != EnabledPerceptions.end() && it->second;
        }
        bool IsHearingEnabled() const 
        { 
            auto it = EnabledPerceptions.find(PercaptionType::Hearing);
            return it != EnabledPerceptions.end() && it->second;
        }
        void SetSightEnabled(bool enabled) { EnabledPerceptions[PercaptionType::Sight] = enabled; }
        void SetHearingEnabled(bool enabled) { EnabledPerceptions[PercaptionType::Hearing] = enabled; }
    };
    struct PerceivableComponent
    {
        std::vector<PerceivableType> Types;
        bool bIsDetectable = true;
        
        std::vector<glm::vec3> DetectablePointsOffsets; // Offsets from the entity's position that can be detected by AI controllers
        
        int DetectionPriority = 0;
        
        PerceivableComponent() = default;
        PerceivableComponent(const PerceivableComponent&) = default;
    };

    struct EntityNameComponent
    {
        std::string Name;

        EntityNameComponent() = default;
        EntityNameComponent(const EntityNameComponent&) = default;
        EntityNameComponent(const std::string& name) : Name(name) {}
    };

    struct TagComponent
    {
        std::vector<std::string> Tags;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::vector<std::string>& tags) : Tags(tags) {}
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
        BodyType Type = BodyType::Static;
        bool FixedRotation = false;

        float Friction = 0.05f;
        float Restitution = 0.0f;
        float ConvexRadius = 0.02f;

        bool lockPositionX = false;
        bool lockPositionY = false;
        bool lockPositionZ = false;
        
        bool lockRotationX = false;
        bool lockRotationY = false;
        bool lockRotationZ = false;

        // Runtime
        void* RuntimeBody = nullptr;

        Rigidbody3DComponent() = default;
        Rigidbody3DComponent(const Rigidbody3DComponent&) = default;
    };
    struct BoxCollider3DComponent
    {
        bool bIsTrigger = false;
        
        glm::vec3 Offset = {0.0f, 0.0f, 0.0f};
        glm::vec3 Size = {0.5f, 0.5f, 0.5f};
        
        void* RuntimeBody = nullptr;
        
        BoxCollider3DComponent() = default;
        BoxCollider3DComponent(const BoxCollider3DComponent&) = default;
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
        EntityNameComponent,
        ChildrenManagerComponent,
        TagComponent,
        LightComponent,
        TextComponent,
        CameraComponent,
        SpriteRendererComponent,
        MeshRendererComponent,
        BehaviorTreeComponent,
        AIControllerComponent,
        PerceivableComponent,
        CircleRendererComponent,
        NativeScriptComponent,
        ScriptComponent,
        Rigidbody2DComponent,
        Rigidbody3DComponent,
        BoxCollider3DComponent,
        BoxCollider2DComponent,
        CircleCollider2DComponent>;
}
