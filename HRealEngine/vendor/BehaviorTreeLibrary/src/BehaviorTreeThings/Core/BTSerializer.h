#pragma once
#include "Tree.h"
#include "yaml-cpp/emitter.h"

class BTSerializer
{
public:
    BTSerializer();
    BTSerializer(BehaviorTree*& tree);
    ~BTSerializer() = default;

    void CreateBehaviorTreeFile(const std::string& filepath);
    void Serialize(const std::string& filepath);
    
    bool Deserialize(const std::string& filepath);
    bool Deserialize(const std::string& filepath, NodeEditorApp* editorAppFromEditor);
    bool Deserialize(const YAML::Node& data);
    bool Deserialize(const YAML::Node& data, NodeEditorApp* editorAppFromEditor);
    bool DeserializeData(const YAML::Node& data);
    bool DeserializeData(const YAML::Node& data, NodeEditorApp* editorApp, const std::string& treeName = "BehaviorTree");

    static bool DeserializeEditorGraphOnly(const YAML::Node& data, NodeEditorApp* editorApp);
private:
    static void CollectRuntimeNodesByUID(const HNode* node, std::unordered_map<uint64_t, const HNode*>& out);
    static const char* NodeTypeToString(HNodeType type);
    static const char* PriorityToString(PriorityType p);

    static void SerializeBlackboard(YAML::Emitter& out, const HBlackboard* blackboard);
    static void DeserializeBlackboard(const YAML::Node& blackboardNode, std::unique_ptr<HBlackboard>& blackboard);
    static void DeserializeBlackboard(const YAML::Node& blackboardNode, HBlackboard* blackboard);

    static void SerializeEditorData(YAML::Emitter& out);

    static void SerializeChildren(YAML::Emitter& out, const HNode* node);
    static void SerializeNode(YAML::Emitter& out, const HNode* node);
    static void DeserializeNodeRecursive(const YAML::Node& nodeData, BehaviorTreeBuilder& builder);
    static void SerializeConditions(YAML::Emitter& out, const HNode* node);
    static void DeserializeCondition(const YAML::Node& condData, BehaviorTreeBuilder& builder);

    BehaviorTree* m_Tree = nullptr;
};