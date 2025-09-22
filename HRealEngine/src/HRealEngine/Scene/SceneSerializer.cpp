
#include "HRpch.h"
#include "SceneSerializer.h"
#include "HRealEngine/Core/Entity.h"
#include "HRealEngine/Core/Components.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

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
}

namespace HRealEngine
{
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
        HREALENGINE_CORE_DEBUGBREAK(entity.HasComponent<TagComponent>(), "Entity has no tag component");
        
        out << YAML::BeginMap;
        out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();//todo entity ID
        
        if (entity.HasComponent<TagComponent>())
        {
            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap;
            auto& tag = entity.GetComponent<TagComponent>().Tag;
            out << YAML::Key << "Tag" << YAML::Value << tag;
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
        if (entity.HasComponent<SpriteRendererComponent>())
        {
            auto& sprite = entity.GetComponent<SpriteRendererComponent>();
            out << YAML::Key << "SpriteRendererComponent";
            out << YAML::BeginMap;
            out << YAML::Key << "Color" << YAML::Value << sprite.Color;
            if (sprite.Texture)
                out << YAML::Key << "TexturePath" << YAML::Value << sprite.Texture->GetPath();
            out << YAML::Key << "TilingFactor" << YAML::Value << sprite.TilingFactor;
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
        if (entity.HasComponent<Rigidbody2DComponent>())
        {
            out << YAML::Key << "Rigidbody2DComponent";
            out << YAML::BeginMap;
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DTypeToString(rb2d.Type);
            out << YAML::Key << "FixedRotation" << YAML::Value << rb2d.FixedRotation;
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

    void SceneSerializer::Serialize(const std::string& filepath)
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << "Untitled";
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        sceneRef->GetRegistry().view<TagComponent>().each([&](auto entityHandle, auto& tagComponent)
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

    void SceneSerializer::SerializeRuntime(const std::string& filepath)
    {
        
    }

    bool SceneSerializer::Deserialize(const std::string& filepath)
    {
        /*std::ifstream stream(filepath);
        std::stringstream strStream;
        strStream << stream.rdbuf();*/
        //YAML::Node data = YAML::Load(strStream.str());
        YAML::Node data;
        try
        {
            data = YAML::LoadFile(filepath);
        }
        catch (YAML::ParserException e)
        {
            return false;
        }
        
        if (!data["Scene"])
            return false;
        std::string sceneName = data["Scene"].as<std::string>();
        
        auto entities = data["Entities"];
        if (entities)
        {
            for (auto entity : entities)
            {
                uint64_t uuid = entity["Entity"].as<uint64_t>();
                
                std::string name;
                auto tagComponent = entity["TagComponent"];
                if (tagComponent)
                    name = tagComponent["Tag"].as<std::string>();

                Entity deserializedEntity = sceneRef->CreateEntityWithUUID(uuid,name);
                if (auto transformComponent = entity["TransformComponent"])
                {
                    auto& transform = deserializedEntity.GetComponent<TransformComponent>();
                    transform.Position = transformComponent["Position"].as<glm::vec3>();
                    transform.Rotation = transformComponent["Rotation"].as<glm::vec3>();
                    transform.Scale = transformComponent["Scale"].as<glm::vec3>();
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
                if (auto spriteRendererComponent = entity["SpriteRendererComponent"])
                {
                    auto& sprite = deserializedEntity.AddComponent<SpriteRendererComponent>();
                    sprite.Color = spriteRendererComponent["Color"].as<glm::vec4>();
                    if (spriteRendererComponent["TexturePath"])
                        sprite.Texture = Texture2D::Create(spriteRendererComponent["TexturePath"].as<std::string>());
                    if (spriteRendererComponent["TilingFactor"])
                        sprite.TilingFactor = spriteRendererComponent["TilingFactor"].as<float>();
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

    bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
    {
        return false;
    }
}
