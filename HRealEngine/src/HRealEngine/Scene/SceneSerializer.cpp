
#include "HRpch.h"
#include "SceneSerializer.h"
#include "HRealEngine/Core/Entity.h"
#include "HRealEngine/Core/Components.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

#include "HRealEngine/Asset/AssetManager.h"
#include "HRealEngine/Core/MeshLoader.h"
#include "HRealEngine/Renderer/Material.h"
#include "HRealEngine/Scripting/ScriptEngine.h"

namespace YAML
{
    template<>
    struct convert<glm::vec2>
    {
        static Node encode(const glm::vec2& vec)
        {
            Node node;
            node.push_back(vec.x);
            node.push_back(vec.y);
            return node;
        }
        static bool decode(const Node& node, glm::vec2& vec)
        {
            if (!node.IsSequence() || node.size() != 2)
                return false;
            vec.x = node[0].as<float>();
            vec.y = node[1].as<float>();
            return true;
        }
    };
    template<>
    struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3& vec)
        {
            Node node;
            node.push_back(vec.x);
            node.push_back(vec.y);
            node.push_back(vec.z);
            return node;
        }
        static bool decode(const Node& node, glm::vec3& vec)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;
            vec.x = node[0].as<float>();
            vec.y = node[1].as<float>();
            vec.z = node[2].as<float>();
            return true;
        }
    };
    template<>
    struct convert<glm::vec4>
    {
        static Node encode(const glm::vec4& vec)
        {
            Node node;
            node.push_back(vec.x);
            node.push_back(vec.y);
            node.push_back(vec.z);
            node.push_back(vec.w);
            return node;
        }
        static bool decode(const Node& node, glm::vec4& vec)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;
            vec.x = node[0].as<float>();
            vec.y = node[1].as<float>();
            vec.z = node[2].as<float>();
            vec.w = node[3].as<float>();
            return true;
        }
    };

    template<>
    struct convert<HRealEngine::UUID>
    {
        static Node encode(const HRealEngine::UUID& uuid)
        {
            Node node;
            node.push_back((uint64_t)uuid);
            return node;
        }
        static bool decode(const Node& node, HRealEngine::UUID& uuid)
        {
            uuid = node.as<uint64_t>();
            return true;
        }
    };
}

namespace HRealEngine
{
#define WRITE_SCRIPT_FIELD(FieldType, Type) \
    case ScriptFieldType::FieldType: \
        out << fieldInstance.GetValue<Type>(); \
        break;
#define READ_SCRIPT_FIELD(FieldType, Type) \
    case ScriptFieldType::FieldType: \
    {                               \
        Type data = scriptField["Data"].as<Type>(); \
        fieldInstance.SetValue(data); \
        break; \
    }
    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& vec)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << vec.x << vec.y << YAML::EndSeq;
        return out;
    }
    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& vec)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << vec.x << vec.y << vec.z << YAML::EndSeq;
        return out;
    }
    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& vec)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << vec.x << vec.y << vec.z << vec.w << YAML::EndSeq;
        return out;
    }

    static std::string RigidBody2DTypeToString(Rigidbody2DComponent::BodyType type)
    {
        switch (type)
        {
            case Rigidbody2DComponent::BodyType::Static:   return "Static";
            case Rigidbody2DComponent::BodyType::Dynamic:  return "Dynamic";
            case Rigidbody2DComponent::BodyType::Kinematic: return "Kinematic";
        }
        HREALENGINE_CORE_DEBUGBREAK(false, "Unknown body type");
        return {};
    }
    static Rigidbody2DComponent::BodyType StringToRigidBody2DType(const std::string& type)
    {
        if (type == "Static")   return Rigidbody2DComponent::BodyType::Static;
        if (type == "Dynamic")  return Rigidbody2DComponent::BodyType::Dynamic;
        if (type == "Kinematic") return Rigidbody2DComponent::BodyType::Kinematic;
        HREALENGINE_CORE_DEBUGBREAK(false, "Unknown body type");
        return Rigidbody2DComponent::BodyType::Static;
    }
    
    SceneSerializer::SceneSerializer(const Ref<Scene>& scene) : sceneRef(scene)
    {
        
    }

    static void SerializeEntity(YAML::Emitter& out, Entity entity)
    {
        HREALENGINE_CORE_DEBUGBREAK(entity.HasComponent<EntityNameComponent>(), "Entity has no tag component");
        
        out << YAML::BeginMap;
        out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();//todo entity ID
        
        if (entity.HasComponent<EntityNameComponent>())
        {
            out << YAML::Key << "EntityNameComponent";
            out << YAML::BeginMap;
            auto& name = entity.GetComponent<EntityNameComponent>().Name;
            out << YAML::Key << "Name" << YAML::Value << name;
            out << YAML::EndMap;
        }
        if (entity.HasComponent<TagComponent>())
        {
            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap;
            auto& tags = entity.GetComponent<TagComponent>().Tags;
            out << YAML::Key << "Tags" << YAML::Value << YAML::BeginSeq;
            for (const std::string& tag : tags)
                out << tag;
            out << YAML::EndSeq;
            out << YAML::EndMap;
        }
        if (entity.HasComponent<TransformComponent>())
        {
            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap;
            auto& transform = entity.GetComponent<TransformComponent>();
            out << YAML::Key << "Position" << YAML::Value << YAML::Flow << YAML::BeginSeq << transform.Position.x << transform.Position.y << transform.Position.z << YAML::EndSeq;
            out << YAML::Key << "Rotation" << YAML::Value << YAML::Flow << YAML::BeginSeq << transform.Rotation.x << transform.Rotation.y << transform.Rotation.z << YAML::EndSeq;
            out << YAML::Key << "Scale" << YAML::Value << YAML::Flow << YAML::BeginSeq << transform.Scale.x << transform.Scale.y << transform.Scale.z << YAML::EndSeq;
            out << YAML::EndMap;
        }
        if (entity.HasComponent<LightComponent>())
        {
            out << YAML::Key << "LightComponent";
            out << YAML::BeginMap;
            auto& light = entity.GetComponent<LightComponent>();
            std::string type;
            switch (light.Type)
            {
                case LightComponent::LightType::Directional: type = "Directional"; break;
                case LightComponent::LightType::Point: type = "Point"; break;
                case LightComponent::LightType::Spot: type = "Spot"; break;
            }
            out << YAML::Key << "Type" << YAML::Value << type;
            out << YAML::Key << "Color" << YAML::Value << light.Color;
            out << YAML::Key << "Direction" << YAML::Value << light.Direction;
            out << YAML::Key << "Intensity" << YAML::Value << light.Intensity;
            out << YAML::Key << "Radius" << YAML::Value << light.Radius;
            out << YAML::Key << "CastShadows" << YAML::Value << light.CastShadows;
            out << YAML::EndMap;
        }
        if (entity.HasComponent<TextComponent>())
        {
            out << YAML::Key << "TextComponent";
            out << YAML::BeginMap;

            auto& textComponent = entity.GetComponent<TextComponent>();
            out << YAML::Key << "TextString" << YAML::Value << textComponent.TextString;
            out << YAML::Key << "Color" << YAML::Value << textComponent.Color;
            out << YAML::Key << "Kerning" << YAML::Value << textComponent.Kerning;
            out << YAML::Key << "LineSpacing" << YAML::Value << textComponent.LineSpacing;

            out << YAML::EndMap;
        }
        if (entity.HasComponent<SpriteRendererComponent>())
        {
            auto& sprite = entity.GetComponent<SpriteRendererComponent>();
            out << YAML::Key << "SpriteRendererComponent";
            out << YAML::BeginMap;
            out << YAML::Key << "Color" << YAML::Value << sprite.Color;
            /*if (sprite.Texture)
                out << YAML::Key << "TexturePath" << YAML::Value << sprite.Texture->GetPath();*/
            out << YAML::Key << "TextureHandle" << YAML::Value << sprite.Texture;
            out << YAML::Key << "TilingFactor" << YAML::Value << sprite.TilingFactor;
            out << YAML::Key << "OrderInLayer" << YAML::Value << sprite.OrderInLayer;
            out << YAML::EndMap;
        }
        if (entity.HasComponent<MeshRendererComponent>())
        {
            auto& mesh = entity.GetComponent<MeshRendererComponent>();
            out << YAML::Key << "MeshRendererComponent";
            out << YAML::BeginMap;

            out << YAML::Key << "PivotOffset" << YAML::Value << mesh.PivotOffset;
            /*if (mesh.MeshAssetPath.empty() == false)
                out << YAML::Key << "MeshPath" << YAML::Value << mesh.MeshAssetPath.string();*/
            out << YAML::Key << "MeshHandle" << YAML::Value << mesh.Mesh;
            
            out << YAML::Key << "Color" << YAML::Value << mesh.Color;
            /*if (mesh.Texture)
                out << YAML::Key << "TexturePath" << YAML::Value << mesh.Texture->GetPath();*/
            out << YAML::Key << "TextureHandle" << YAML::Value << mesh.Texture;
            out << YAML::Key << "TilingFactor" << YAML::Value << mesh.TilingFactor;

            out << YAML::Key << "MaterialHandleOverrides";
            out << YAML::Value << YAML::BeginSeq;
            for (AssetHandle h : mesh.MaterialHandleOverrides)
            {
                out << h;
                if (h == 0 || !AssetManager::IsAssetHandleValid(h))
                    continue;
                auto material = AssetManager::GetAsset<HMaterial>(h);
                if (material)
                    material->SaveToFile();
            }
            out << YAML::EndSeq;
            
            out << YAML::EndMap;
        }
        if (entity.HasComponent<BehaviorTreeComponent>())
        {
            out << YAML::Key << "BehaviorTreeComponent";
            out << YAML::BeginMap;
            auto& bt = entity.GetComponent<BehaviorTreeComponent>();
            out << YAML::Key << "BehaviorTreeHandle" << YAML::Value << bt.BehaviorTreeAsset;
            out << YAML::EndMap;
        }
        if (entity.HasComponent<AIControllerComponent>())
        {
            out << YAML::Key << "AIControllerComponent";
            out << YAML::BeginMap;
            auto& ai = entity.GetComponent<AIControllerComponent>();
            
            out << YAML::Key << "UpdateInterval" << YAML::Value << ai.UpdateInterval;
            
            out << YAML::Key << "SightEnabled" << YAML::Value << ai.EnabledPerceptions[PercaptionType::Sight];
            out << YAML::Key << "HearingEnabled" << YAML::Value << ai.EnabledPerceptions[PercaptionType::Hearing];
            
            out << YAML::Key << "SightConfig";
            out << YAML::BeginMap;
            out << YAML::Key << "SightRadius" << YAML::Value << ai.SightSettings.SightRadius;
            out << YAML::Key << "FieldOfView" << YAML::Value << ai.SightSettings.FieldOfView;
            out << YAML::Key << "ForgetDuration" << YAML::Value << ai.SightSettings.ForgetDuration;
            out << YAML::Key << "DetectableTypes" << YAML::Value << YAML::BeginSeq;
            for (auto type : ai.SightSettings.DetectableTypes)
                out << (int)type;
            out << YAML::EndSeq;
            out << YAML::EndMap;
            
            out << YAML::Key << "HearingConfig";
            out << YAML::BeginMap;
            out << YAML::Key << "HearingRadius" << YAML::Value << ai.HearingSettings.HearingRadius;
            out << YAML::Key << "ForgetDuration" << YAML::Value << ai.HearingSettings.ForgetDuration;
            out << YAML::Key << "DetectableTypes" << YAML::Value << YAML::BeginSeq;
            for (auto type : ai.HearingSettings.DetectableTypes)
                out << (int)type;
            out << YAML::EndSeq;
            out << YAML::EndMap;
            
            out << YAML::EndMap;
        }
        if (entity.HasComponent<PerceivableComponent>())
        {
            out << YAML::Key << "PerceivableComponent";
            out << YAML::BeginMap;
            auto& perc = entity.GetComponent<PerceivableComponent>();
            
            out << YAML::Key << "IsDetectable" << YAML::Value << perc.bIsDetectable;
            out << YAML::Key << "DetectionPriority" << YAML::Value << perc.DetectionPriority;
            
            out << YAML::Key << "Types" << YAML::Value << YAML::BeginSeq;
            for (auto type : perc.Types)
                out << (int)type;
            out << YAML::EndSeq;
            
            out << YAML::Key << "DetectablePointsOffsets" << YAML::Value << YAML::BeginSeq;
            for (auto& offset : perc.DetectablePointsOffsets)
                out << offset;
            out << YAML::EndSeq;
            
            out << YAML::EndMap;
        }
        if (entity.HasComponent<CircleRendererComponent>())
        {
            auto& circle = entity.GetComponent<CircleRendererComponent>();
            out << YAML::Key << "CircleRendererComponent";
            out << YAML::BeginMap;
            out << YAML::Key << "Color" << YAML::Value << YAML::Flow << YAML::BeginSeq << circle.Color.r << circle.Color.g << circle.Color.b << circle.Color.a << YAML::EndSeq;
            out << YAML::Key << "Thickness" << YAML::Value << circle.Thickness;
            out << YAML::Key << "Fade" << YAML::Value << circle.Fade;
            out << YAML::EndMap;
        }
        if (entity.HasComponent<CameraComponent>())
        {
            out << YAML::Key << "CameraComponent";
            out << YAML::BeginMap;
            
            auto& cameraComponent = entity.GetComponent<CameraComponent>();
            auto& camera = cameraComponent.Camera;
            
            out << YAML::Key << "PrimaryCamera" << YAML::Value << cameraComponent.PrimaryCamera;
            out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;
            out << YAML::Key << "Camera" << YAML::Value;
            out << YAML::BeginMap;
            out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
            out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveFOV();
            out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNear();
            out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFar();
            out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
            out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetNearClip();
            out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetFarClip();
            out << YAML::EndMap;
            out << YAML::EndMap;
        }
        if (entity.HasComponent<ScriptComponent>())
        {
            auto& scriptComponent = entity.GetComponent<ScriptComponent>();
            out << YAML::Key << "ScriptComponent";
            out << YAML::BeginMap;
            out << YAML::Key << "ClassName" << YAML::Value << scriptComponent.ClassName;

            Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(scriptComponent.ClassName);
            const auto& fields = entityClass->GetFields();
            if (fields.size() > 0)
            {
                out << YAML::Key << "ScriptFields" << YAML::Value;
                auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);
                out << YAML::BeginSeq;
                for (const auto& [name, field] : fields)
                {
                    if (entityFields.find(name) == entityFields.end())
                        continue;
                    out << YAML::BeginMap;
                    out << YAML::Key << "Name" << YAML::Value << name;
                    out << YAML::Key << "Type" << YAML::Value << ScriptEngine::ScriptFieldTypeToString(field.Type);

                    out << YAML::Key << "Data" << YAML::Value;
                    ScriptFieldInstance& fieldInstance = entityFields.at(name);

                    switch (field.Type)
                    {
                        WRITE_SCRIPT_FIELD(Float, float)
                        WRITE_SCRIPT_FIELD(Double, double)
                        WRITE_SCRIPT_FIELD(Bool, bool)
                        WRITE_SCRIPT_FIELD(Char, char)
                        WRITE_SCRIPT_FIELD(Byte, int8_t)
                        WRITE_SCRIPT_FIELD(Short, int16_t)
                        WRITE_SCRIPT_FIELD(Int, int32_t)
                        WRITE_SCRIPT_FIELD(Long, int64_t)
                        WRITE_SCRIPT_FIELD(UByte, uint8_t)
                        WRITE_SCRIPT_FIELD(UShort, uint16_t)
                        WRITE_SCRIPT_FIELD(UInt, uint32_t)
                        WRITE_SCRIPT_FIELD(ULong, uint64_t)
                        WRITE_SCRIPT_FIELD(Vector2, glm::vec2)
                        WRITE_SCRIPT_FIELD(Vector3, glm::vec3);
                        WRITE_SCRIPT_FIELD(Vector4, glm::vec4)
                        WRITE_SCRIPT_FIELD(Entity, UUID)
                        WRITE_SCRIPT_FIELD(String, std::string)
                    }
                    out << YAML::EndMap;
                }
                out << YAML::EndSeq;
            }
            out << YAML::EndMap;
        }
        if (entity.HasComponent<Rigidbody2DComponent>())
        {
            out << YAML::Key << "Rigidbody2DComponent";
            out << YAML::BeginMap;
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DTypeToString(rb2d.Type);
            out << YAML::Key << "FixedRotation" << YAML::Value << rb2d.FixedRotation;
            out << YAML::EndMap;
        }
        if (entity.HasComponent<Rigidbody3DComponent>())
        {
            out << YAML::Key << "Rigidbody3DComponent";
            out << YAML::BeginMap;
            auto& rb3d = entity.GetComponent<Rigidbody3DComponent>();
            std::string bodyType;
            switch (rb3d.Type)
            {
                case Rigidbody3DComponent::BodyType::Static:   bodyType = "Static"; break;
                case Rigidbody3DComponent::BodyType::Dynamic:  bodyType = "Dynamic"; break;
                case Rigidbody3DComponent::BodyType::Kinematic: bodyType = "Kinematic"; break;
            }
            out << YAML::Key << "BodyType" << YAML::Value << bodyType;
            out << YAML::Key << "LockPositionX" << YAML::Value << rb3d.lockPositionX;
            out << YAML::Key << "LockPositionY" << YAML::Value << rb3d.lockPositionY;
            out << YAML::Key << "LockPositionZ" << YAML::Value << rb3d.lockPositionZ;
            out << YAML::Key << "LockRotationX" << YAML::Value << rb3d.lockRotationX;
            out << YAML::Key << "LockRotationY" << YAML::Value << rb3d.lockRotationY;
            out << YAML::Key << "LockRotationZ" << YAML::Value << rb3d.lockRotationZ;
            out << YAML::Key << "FixedRotation" << YAML::Value << rb3d.FixedRotation;
            out << YAML::Key << "Friction" << YAML::Value << rb3d.Friction;
            out << YAML::Key << "Restitution" << YAML::Value << rb3d.Restitution;
            out << YAML::Key << "Convex Radius" << YAML::Value << rb3d.ConvexRadius;
            out << YAML::EndMap;
        }
        if (entity.HasComponent<BoxCollider3DComponent>())
        {
            out << YAML::Key << "BoxCollider3DComponent";
            out << YAML::BeginMap;
            auto& bc3d = entity.GetComponent<BoxCollider3DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << bc3d.Offset;
            out << YAML::Key << "Size" << YAML::Value << bc3d.Size;
            out << YAML::Key << "IsTrigger" << YAML::Value << bc3d.bIsTrigger;
            out << YAML::EndMap;
        }
        if (entity.HasComponent<BoxCollider2DComponent>())
        {
            out << YAML::Key << "BoxCollider2DComponent";
            out << YAML::BeginMap;
            auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << bc2d.Offset;
            out << YAML::Key << "Size" << YAML::Value << bc2d.Size;
            out << YAML::Key << "Density" << YAML::Value << bc2d.Density;
            out << YAML::Key << "Friction" << YAML::Value << bc2d.Friction;
            out << YAML::Key << "Restitution" << YAML::Value << bc2d.Restitution;
            out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2d.RestitutionThreshold;
            
            out << YAML::EndMap;
        }
        if (entity.HasComponent<CircleCollider2DComponent>())
        {
            out << YAML::Key << "CircleCollider2DComponent";
            out << YAML::BeginMap;
            auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << cc2d.Offset;
            out << YAML::Key << "Radius" << YAML::Value << cc2d.Radius;
            out << YAML::Key << "Density" << YAML::Value << cc2d.Density;
            out << YAML::Key << "Friction" << YAML::Value << cc2d.Friction;
            out << YAML::Key << "Restitution" << YAML::Value << cc2d.Restitution;
            out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2d.RestitutionThreshold;
            out << YAML::EndMap;
        }
        
        out << YAML::EndMap;
    }

    void SceneSerializer::Serialize(const std::filesystem::path& filepath)
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << sceneRef->GetSceneName();
        out << YAML::Key << "SceneSettings";
        out << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "Physic" << YAML::Value << (sceneRef->Is2DPhysicsEnabled() ? "2D" : "3D");
        out << YAML::EndMap;
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        sceneRef->GetRegistry().view<EntityNameComponent>().each([&](auto entityHandle, auto& nameComponent)
        {
            Entity entity{entityHandle, sceneRef.get()};
            if (!entity)
                return;
            SerializeEntity(out, entity);
        });
        out << YAML::EndSeq;
        out << YAML::EndMap;
        std::ofstream fout(filepath);
        fout << out.c_str();
    }

    void SceneSerializer::SerializeRuntime(const std::filesystem::path& filepath)
    {
        
    }

    bool SceneSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        /*std::ifstream stream(filepath);
        std::stringstream strStream;
        strStream << stream.rdbuf();*/
        //YAML::Node data = YAML::Load(strStream.str());
        YAML::Node data;
        try
        {
            data = YAML::LoadFile(filepath.string());
        }
        catch (YAML::ParserException e)
        {
            HREALENGINE_CORE_DEBUGBREAK("Failed to load scene file '{0}'\n     {1}", filepath, e.what());
            return false;
        }
        
        if (!data["Scene"])
            return false;
        std::string sceneName = data["Scene"].as<std::string>();
        sceneRef->SetSceneName(sceneName);
        
        bool bIs2D = false;
        auto sceneSettings = data["SceneSettings"];
        if (sceneSettings)        
        {
            std::string physic = sceneSettings["Physic"].as<std::string>();
            bIs2D = physic == "2D";
        }
        sceneRef->Set2DPhysicsEnabled(bIs2D);
        
        auto entities = data["Entities"];
        if (entities)
        {
            for (auto entity : entities)
            {
                uint64_t uuid = entity["Entity"].as<uint64_t>();
                
                std::string name;
                auto nameComponent = entity["EntityNameComponent"];
                if (nameComponent)
                    name = nameComponent["Name"].as<std::string>();

                Entity deserializedEntity = sceneRef->CreateEntityWithUUID(uuid,name);
                if (auto tagComponent = entity["TagComponent"])
                {
                    auto& tag = deserializedEntity.AddComponent<TagComponent>();
                    auto tags = tagComponent["Tags"];
                    for (auto tagNode : tags)
                        tag.Tags.push_back(tagNode.as<std::string>());
                }
                if (auto transformComponent = entity["TransformComponent"])
                {
                    auto& transform = deserializedEntity.GetComponent<TransformComponent>();
                    transform.Position = transformComponent["Position"].as<glm::vec3>();
                    transform.Rotation = transformComponent["Rotation"].as<glm::vec3>();
                    transform.Scale = transformComponent["Scale"].as<glm::vec3>();
                }
                if (auto lightComponent = entity["LightComponent"])
                {
                    auto& light = deserializedEntity.AddComponent<LightComponent>();
                    std::string type = lightComponent["Type"].as<std::string>();
                    if (type == "Directional")
                        light.Type = LightComponent::LightType::Directional;
                    else if (type == "Point")
                        light.Type = LightComponent::LightType::Point;
                    else if (type == "Spot")
                        light.Type = LightComponent::LightType::Spot;
                    light.Color = lightComponent["Color"].as<glm::vec3>();
                    light.Direction = lightComponent["Direction"].as<glm::vec3>();
                    light.Intensity = lightComponent["Intensity"].as<float>();
                    light.Radius = lightComponent["Radius"].as<float>();
                    light.CastShadows = lightComponent["CastShadows"].as<bool>();
                }
                if (auto textComponent = entity["TextComponent"])
                {
                    auto& text = deserializedEntity.AddComponent<TextComponent>();
                    text.TextString = textComponent["TextString"].as<std::string>();
                    text.Color = textComponent["Color"].as<glm::vec4>();
                    text.Kerning = textComponent["Kerning"].as<float>();
                    text.LineSpacing = textComponent["LineSpacing"].as<float>();
                }
                if (auto cameraComponent = entity["CameraComponent"])
                {
                    auto& cameraComp = deserializedEntity.AddComponent<CameraComponent>();
                    
                    auto& camera = cameraComp.Camera;
                    cameraComp.PrimaryCamera = cameraComponent["PrimaryCamera"].as<bool>();
                    cameraComp.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
                    
                    auto cameraProps = cameraComponent["Camera"];
                    camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());
                    
                    camera.SetPerspectiveFOV(cameraProps["PerspectiveFOV"].as<float>());
                    camera.SetPerspectiveNear(cameraProps["PerspectiveNear"].as<float>());
                    camera.SetPerspectiveFar(cameraProps["PerspectiveFar"].as<float>());
                    
                    camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
                    camera.SetNearClip(cameraProps["OrthographicNear"].as<float>());
                    camera.SetFarClip(cameraProps["OrthographicFar"].as<float>());

                    cameraComp.PrimaryCamera = cameraComponent["PrimaryCamera"].as<bool>();
                    cameraComp.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
                }
                if (auto scriptComponent = entity["ScriptComponent"])
                {
                    auto& sc = deserializedEntity.AddComponent<ScriptComponent>();
                    
                    sc.ClassName = scriptComponent["ClassName"].as<std::string>();
                    auto scriptFields = scriptComponent["ScriptFields"];
                    if (scriptFields)
                    {
                        Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.ClassName);
                        if (entityClass)
                        {
                            const auto& fields = entityClass->GetFields();
                            auto& entityFields = ScriptEngine::GetScriptFieldMap(deserializedEntity);
                            for (auto scriptField : scriptFields)
                            {   
                                std::string name = scriptField["Name"].as<std::string>();//as?
                                std::string typeString = scriptField["Type"].as<std::string>();
                                ScriptFieldType type = ScriptEngine::ScriptFieldTypeFromString(typeString);
                                ScriptFieldInstance& fieldInstance = entityFields[name];
                                if (fields.find(name) == fields.end())
                                    continue;
                                fieldInstance.Field = fields.at(name);//at?
                                switch (type)
                                {
                                    READ_SCRIPT_FIELD(Float, float)
                                    READ_SCRIPT_FIELD(Double, double)
                                    READ_SCRIPT_FIELD(Bool, bool)
                                    READ_SCRIPT_FIELD(Char, char)
                                    READ_SCRIPT_FIELD(Byte, int8_t)
                                    READ_SCRIPT_FIELD(Short, int16_t)
                                    READ_SCRIPT_FIELD(Int, int32_t)
                                    READ_SCRIPT_FIELD(Long, int64_t)
                                    READ_SCRIPT_FIELD(UByte, uint8_t)
                                    READ_SCRIPT_FIELD(UShort, uint16_t)
                                    READ_SCRIPT_FIELD(UInt, uint32_t)
                                    READ_SCRIPT_FIELD(ULong, uint64_t)
                                    READ_SCRIPT_FIELD(Vector2, glm::vec2)
                                    READ_SCRIPT_FIELD(Vector3, glm::vec3)
                                    READ_SCRIPT_FIELD(Vector4, glm::vec4)
                                    READ_SCRIPT_FIELD(Entity, UUID)
                                    READ_SCRIPT_FIELD(String, std::string)
                                }
                            }
                        }
                    }
                }
                if (auto spriteRendererComponent = entity["SpriteRendererComponent"])
                {
                    auto& sprite = deserializedEntity.AddComponent<SpriteRendererComponent>();
                    sprite.Color = spriteRendererComponent["Color"].as<glm::vec4>();
                    if (spriteRendererComponent["TexturePath"])
                    {
                        //sprite.Texture = Texture2D::Create(spriteRendererComponent["TexturePath"].as<std::string>());
                    }
                    if (spriteRendererComponent["TextureHandle"])
                    {
                        sprite.Texture = spriteRendererComponent["TextureHandle"].as<AssetHandle>();
                    }
                    if (spriteRendererComponent["TilingFactor"])
                        sprite.TilingFactor = spriteRendererComponent["TilingFactor"].as<float>();
                    if (spriteRendererComponent["OrderInLayer"])
                        sprite.OrderInLayer = spriteRendererComponent["OrderInLayer"].as<int>();
                }
                if (auto meshRendererComponent = entity["MeshRendererComponent"])
                {
                    auto& mesh = deserializedEntity.AddComponent<MeshRendererComponent>();

                    mesh.PivotOffset = meshRendererComponent["PivotOffset"].as<glm::vec3>();
                    
                    mesh.Color = meshRendererComponent["Color"].as<glm::vec4>();
                
                    if (meshRendererComponent["TextureHandle"])
                        mesh.Texture = meshRendererComponent["TextureHandle"].as<AssetHandle>();
                
                    if (meshRendererComponent["TilingFactor"])
                        mesh.TilingFactor = meshRendererComponent["TilingFactor"].as<float>();
                    
                    if (meshRendererComponent["MeshHandle"])
                    {
                        mesh.Mesh = meshRendererComponent["MeshHandle"].as<AssetHandle>();
                    }
                    
                    mesh.MaterialHandleOverrides.clear();
                
                    if (meshRendererComponent["MaterialHandleOverrides"] && meshRendererComponent["MaterialHandleOverrides"].IsSequence())
                    {
                        for (auto n : meshRendererComponent["MaterialHandleOverrides"])
                            mesh.MaterialHandleOverrides.push_back(n.as<AssetHandle>());
                    }
                    
                    if (mesh.Mesh != 0 && AssetManager::IsAssetHandleValid(mesh.Mesh))
                    {
                        Ref<MeshGPU> meshGPU = AssetManager::GetAsset<MeshGPU>(mesh.Mesh);
                        if (meshGPU)
                        {
                            const size_t slotCount = meshGPU->MaterialHandles.size();
                            if (slotCount > 0)
                            {
                                if (mesh.MaterialHandleOverrides.size() < slotCount)
                                    mesh.MaterialHandleOverrides.resize(slotCount, 0);
                
                                for (size_t i = 0; i < slotCount; i++)
                                {
                                    if (mesh.MaterialHandleOverrides[i] == 0)
                                        mesh.MaterialHandleOverrides[i] = meshGPU->MaterialHandles[i];
                                }
                            }
                        }
                    }
                }
                if (auto btComponent = entity["BehaviorTreeComponent"])
                {
                    auto& bt = deserializedEntity.AddComponent<BehaviorTreeComponent>();
                    if (btComponent["BehaviorTreeHandle"])
                        bt.BehaviorTreeAsset = btComponent["BehaviorTreeHandle"].as<AssetHandle>();
                }
                if (auto aiComponent = entity["AIControllerComponent"])
                {
                    auto& ai = deserializedEntity.AddComponent<AIControllerComponent>();
                    
                    if (aiComponent["UpdateInterval"])
                        ai.UpdateInterval = aiComponent["UpdateInterval"].as<float>();
                    
                    ai.EnabledPerceptions[PercaptionType::Sight] = aiComponent["SightEnabled"].as<bool>();
                    ai.EnabledPerceptions[PercaptionType::Hearing] = aiComponent["HearingEnabled"].as<bool>();
                    
                    if (auto sightCfg = aiComponent["SightConfig"])
                    {
                        ai.SightSettings.SightRadius = sightCfg["SightRadius"].as<float>();
                        ai.SightSettings.FieldOfView = sightCfg["FieldOfView"].as<float>();
                        ai.SightSettings.ForgetDuration = sightCfg["ForgetDuration"].as<float>();
                        if (sightCfg["DetectableTypes"] && sightCfg["DetectableTypes"].IsSequence())
                        {
                            for (auto n : sightCfg["DetectableTypes"])
                                ai.SightSettings.DetectableTypes.push_back((PerceivableType)n.as<int>());
                        }
                    }
                    
                    if (auto hearingCfg = aiComponent["HearingConfig"])
                    {
                        ai.HearingSettings.HearingRadius = hearingCfg["HearingRadius"].as<float>();
                        ai.HearingSettings.ForgetDuration = hearingCfg["ForgetDuration"].as<float>();
                        if (hearingCfg["DetectableTypes"] && hearingCfg["DetectableTypes"].IsSequence())
                        {
                            for (auto n : hearingCfg["DetectableTypes"])
                                ai.HearingSettings.DetectableTypes.push_back((PerceivableType)n.as<int>());
                        }
                    }
                }
                if (auto percComponent = entity["PerceivableComponent"])
                {
                    auto& perc = deserializedEntity.AddComponent<PerceivableComponent>();
                    
                    perc.bIsDetectable = percComponent["IsDetectable"].as<bool>();
                    perc.DetectionPriority = percComponent["DetectionPriority"].as<int>();
                    
                    if (percComponent["Types"] && percComponent["Types"].IsSequence())
                    {
                        for (auto n : percComponent["Types"])
                            perc.Types.push_back((PerceivableType)n.as<int>());
                    }
                    
                    if (percComponent["DetectablePointsOffsets"] && percComponent["DetectablePointsOffsets"].IsSequence())
                    {
                        for (auto n : percComponent["DetectablePointsOffsets"])
                            perc.DetectablePointsOffsets.push_back(n.as<glm::vec3>());
                    }
                }
                if (auto circleRendererComponent = entity["CircleRendererComponent"])
                {
                    auto& circle = deserializedEntity.AddComponent<CircleRendererComponent>();
                    circle.Color = circleRendererComponent["Color"].as<glm::vec4>();
                    circle.Thickness = circleRendererComponent["Thickness"].as<float>();
                    circle.Fade = circleRendererComponent["Fade"].as<float>();
                }
                if (auto rb2dComponent = entity["Rigidbody2DComponent"])
                {
                    auto& rb2d = deserializedEntity.AddComponent<Rigidbody2DComponent>();
                    rb2d.Type = StringToRigidBody2DType(rb2dComponent["BodyType"].as<std::string>());
                    rb2d.FixedRotation = rb2dComponent["FixedRotation"].as<bool>();
                }
                if (auto rb3dComponent = entity["Rigidbody3DComponent"])
                {
                    auto& rb3d = deserializedEntity.AddComponent<Rigidbody3DComponent>();
                    std::string bodyType = rb3dComponent["BodyType"].as<std::string>();
                    if (bodyType == "Static")        rb3d.Type = Rigidbody3DComponent::BodyType::Static;
                    else if (bodyType == "Dynamic")  rb3d.Type = Rigidbody3DComponent::BodyType::Dynamic;
                    else if (bodyType == "Kinematic")rb3d.Type = Rigidbody3DComponent::BodyType::Kinematic;

                    rb3d.lockPositionX = rb3dComponent["LockPositionX"].as<bool>();
                    rb3d.lockPositionY = rb3dComponent["LockPositionY"].as<bool>();
                    rb3d.lockPositionZ = rb3dComponent["LockPositionZ"].as<bool>();
                    rb3d.lockRotationX = rb3dComponent["LockRotationX"].as<bool>();
                    rb3d.lockRotationY = rb3dComponent["LockRotationY"].as<bool>();
                    rb3d.lockRotationZ = rb3dComponent["LockRotationZ"].as<bool>();
                    rb3d.FixedRotation = rb3dComponent["FixedRotation"].as<bool>();
                    rb3d.Friction = rb3dComponent["Friction"].as<float>();
                    rb3d.Restitution = rb3dComponent["Restitution"].as<float>();
                    rb3d.ConvexRadius = rb3dComponent["Convex Radius"].as<float>();
                }
                if (auto boxCollider3DComponent = entity["BoxCollider3DComponent"])
                {
                    auto& bc3d = deserializedEntity.AddComponent<BoxCollider3DComponent>();
                    bc3d.Offset = boxCollider3DComponent["Offset"].as<glm::vec3>();
                    bc3d.Size = boxCollider3DComponent["Size"].as<glm::vec3>();
                    bc3d.bIsTrigger = boxCollider3DComponent["IsTrigger"].as<bool>();
                }
                if (auto boxCollider2DComponent = entity["BoxCollider2DComponent"])
                {
                    auto& bc2d = deserializedEntity.AddComponent<BoxCollider2DComponent>();
                    bc2d.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
                    bc2d.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
                    bc2d.Density = boxCollider2DComponent["Density"].as<float>();
                    bc2d.Friction = boxCollider2DComponent["Friction"].as<float>();
                    bc2d.Restitution = boxCollider2DComponent["Restitution"].as<float>();
                    bc2d.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
                }
                if (auto circleCollider2DComponent = entity["CircleCollider2DComponent"])
                {
                    auto& cc2d = deserializedEntity.AddComponent<CircleCollider2DComponent>();
                    cc2d.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
                    cc2d.Radius = circleCollider2DComponent["Radius"].as<float>();
                    cc2d.Density = circleCollider2DComponent["Density"].as<float>();
                    cc2d.Friction = circleCollider2DComponent["Friction"].as<float>();
                    cc2d.Restitution = circleCollider2DComponent["Restitution"].as<float>();
                    cc2d.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
                }
            }
        }
        return true;
    }

    bool SceneSerializer::DeserializeRuntime(const std::filesystem::path& filepath)
    {
        return false;
    }
}
