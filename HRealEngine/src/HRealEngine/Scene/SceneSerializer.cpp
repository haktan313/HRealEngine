#include "SceneSerializer.h"

#include <fstream>

#include "Components.h"
#include "yaml-cpp/emitter.h"
#include <yaml-cpp/yaml.h>

#include "yaml-cpp/node/parse.h"

namespace YAML
{
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
    SceneSerializer::SceneSerializer(const Ref<Scene>& scene) : sceneRef(scene)
    {
        
    }

    static void SerializeEntity(YAML::Emitter& out, Entity entity)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Entity" << YAML::Value << "12312312312";//todo entity ID
        
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
            out << YAML::Key << "SpriteRendererComponent";
            out << YAML::BeginMap;
            auto& sprite = entity.GetComponent<SpriteRendererComponent>();
            out << YAML::Key << "Color" << YAML::Value << YAML::Flow << YAML::BeginSeq << sprite.Color.r << sprite.Color.g << sprite.Color.b << sprite.Color.a << YAML::EndSeq;
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

                Entity deserializedEntity = sceneRef->CreateEntity(name);
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
