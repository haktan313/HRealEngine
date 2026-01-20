#include "BTSerializer.h"

#include <filesystem>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include "NodeRegistry.h"
#include "Editor/EditorRoot.h"
#include "Editor/NodeEditorApp.h"

BTSerializer::BTSerializer() : m_Tree(nullptr)
{
    
}

BTSerializer::BTSerializer(BehaviorTree*& tree) : m_Tree(tree)
{
}

void BTSerializer::CreateBehaviorTreeFile(const std::string& filepath)
{
    YAML::Emitter out;
    out << YAML::BeginMap;

    out << YAML::Key << "BehaviorTree" << YAML::Value;
    out << YAML::BeginMap;

    out << YAML::Key << "Blackboard" << YAML::Value;
    out << YAML::BeginMap;
    out << YAML::Key << "ClassName" << YAML::Value << "NoBlackboard";
    out << YAML::EndMap;

    out << YAML::Key << "EditorData" << YAML::Value;
    out << YAML::BeginMap;
    SerializeEditorData(out);
    out << YAML::EndMap;

    out << YAML::Key << "RuntimeData" << YAML::Value;
    SerializeNode(out, nullptr);

    out << YAML::EndMap;// BehaviorTree
    out << YAML::EndMap;// root

    std::ofstream fout(filepath);
    fout << out.c_str();
}

void BTSerializer::Serialize(const std::string& filepath)
{
    auto rootNode = m_Tree->GetRootNode();

    YAML::Emitter out;
    out << YAML::BeginMap;

    out << YAML::Key << "BehaviorTree" << YAML::Value;
    out << YAML::BeginMap;

    out << YAML::Key << "Blackboard" << YAML::Value;
    out << YAML::BeginMap;
    SerializeBlackboard(out, m_Tree->GetBlackboardRaw());
    out << YAML::EndMap;

    out << YAML::Key << "EditorData" << YAML::Value;
    out << YAML::BeginMap;
    SerializeEditorData(out);
    out << YAML::EndMap;

    out << YAML::Key << "RuntimeData" << YAML::Value;
    SerializeNode(out, rootNode);

    out << YAML::EndMap;// BehaviorTree
    out << YAML::EndMap;// root

    std::ofstream fout(filepath);
    fout << out.c_str();
}

bool BTSerializer::Deserialize(const std::string& filepath)
{
    YAML::Node data = YAML::LoadFile(filepath);
    return DeserializeData(data);
}

bool BTSerializer::Deserialize(const std::string& filepath, NodeEditorApp* editorApp)
{
    YAML::Node data = YAML::LoadFile(filepath);
    
    std::filesystem::path path(filepath);
    std::string fileName = path.filename().string();
    
    return DeserializeData(data, editorApp, fileName);
}

bool BTSerializer::Deserialize(const YAML::Node& data)
{
    return DeserializeData(data);
}

bool BTSerializer::Deserialize(const YAML::Node& data, NodeEditorApp* editorAppFromEditor)
{
    return DeserializeData(data, editorAppFromEditor);
}

bool BTSerializer::DeserializeData(const YAML::Node& data)
{
    auto btNode = data["BehaviorTree"];
    if (!btNode)
        return false;
    
    std::unique_ptr<HBlackboard> blackboard = nullptr;
    std::string bbClassName = "";

    if (btNode["Blackboard"]["ClassName"])
    {
        bbClassName = btNode["Blackboard"]["ClassName"].as<std::string>();
        auto& bbRegistry = NodeRegistry::GetBlackboardClassInfoMap();
        
        auto it = bbRegistry.find(bbClassName);
        if (it != bbRegistry.end())
            blackboard = it->second.CreateBlackboardFn();
    }
    if (!blackboard)
        blackboard = std::make_unique<HBlackboard>();
    DeserializeBlackboard(btNode["Blackboard"], blackboard);
    
    
    BehaviorTreeBuilder builder(m_Tree);
    builder.setBlackboard(std::move(blackboard));

    DeserializeNodeRecursive(btNode["RuntimeData"], builder);

    builder.build();
    return true;
}

bool BTSerializer::DeserializeData(const YAML::Node& data, NodeEditorApp* editorApp, const std::string& treeName)
{
    auto btNode = data["BehaviorTree"];
    if (!btNode)
        return false;
    
    HBlackboard* blackboard = nullptr;
    std::string bbClassName = "";

    if (btNode["Blackboard"]["ClassName"])
    {
        bbClassName = btNode["Blackboard"]["ClassName"].as<std::string>();
        auto& bbRegistry = NodeRegistry::GetBlackboardClassInfoMap();
        
        auto it = bbRegistry.find(bbClassName);
        if (it != bbRegistry.end())
        {
            if (editorApp)
                blackboard = &editorApp->SetBlackboardForEditor(bbClassName, it->second);
        }
    }
    if (!blackboard)
        blackboard = new HBlackboard();
    DeserializeBlackboard(btNode["Blackboard"], blackboard);
    
    editorApp->ClearBuildData();
    editorApp->GetNodeEditorHelper().ClearDatas();
    
    std::unordered_map<int, nodeEditor::NodeId> idMap;

    int maxID = 0;

    if (btNode["EditorData"]["Nodes"])
    {
        for (auto n : btNode["EditorData"]["Nodes"])
        {
            int oldID = n["ID"].as<int>();
            maxID = std::max(maxID, oldID);
            
            std::string name = n["Name"].as<std::string>();
            NodeType type = static_cast<NodeType>(n["Type"].as<int>());
            ImVec2 pos(n["PosX"].as<float>(), n["PosY"].as<float>());

            Node* newNode = nullptr;

            switch (type)
            {
                case NodeType::Root:
                    newNode = editorApp->GetNodeEditorHelper().SpawnRootNode();
                break;
                case NodeType::Sequence:
                    newNode = editorApp->GetNodeEditorHelper().SpawnSequenceNode(pos);
                break;
                case NodeType::Selector:
                    newNode = editorApp->GetNodeEditorHelper().SpawnSelectorNode(pos);
                break;
                case NodeType::Action:
                    newNode = editorApp->GetNodeEditorHelper().SpawnActionNode(pos);
                break;
            }

            if (newNode)
            {
                newNode->Name = name;
                nodeEditor::SetNodePosition(newNode->ID, pos);
                idMap[oldID] = newNode->ID;
                int nodeKey = (int)newNode->ID.Get();
                
                if (n["Decorators"])
                    for (auto d : n["Decorators"])
                    {
                        std::string className = d["ClassName"].as<std::string>();
                        auto& decoMap = NodeRegistry::GetDecoratorClassInfoMap();
                        if (decoMap.count(className))
                        {
                            editorApp->m_NodeToDecoratorClassId[nodeKey] = className;
                            editorApp->m_NodeToDecoratorParams[nodeKey] = decoMap[className].CreateParamsFn();
                            editorApp->m_NodeToDecoratorParams[nodeKey]->Deserialize(d["Params"]);
                            
                            EditorDecorator edeco(d["Name"].as<std::string>());
                            edeco.ClassName = className;
                            edeco.Params = editorApp->m_NodeToDecoratorParams[nodeKey].get();
                            newNode->Decorators.push_back(edeco);
                        }
                    }
                if (n["Conditions"])
                    for (auto c : n["Conditions"])
                    {
                        std::string className = c["ClassName"].as<std::string>();
                        auto& condMap = NodeRegistry::GetConditionClassInfoMap();
                        if (condMap.count(className))
                        {
                            editorApp->m_NodeToConditionClassId[nodeKey] = className;
                            editorApp->m_NodeToConditionParams[nodeKey] = condMap[className].CreateParamsFn();
                            editorApp->m_NodeToConditionParams[nodeKey]->Deserialize(c["Params"]);
                            
                            if (c["Params"]["Priority"])
                            {
                                std::string pStr = c["Params"]["Priority"].as<std::string>();
                                PriorityType p = PriorityType::None;
                                if (pStr == "Self")
                                    p = PriorityType::Self;
                                else if (pStr == "LowerPriority")
                                    p = PriorityType::LowerPriority;
                                else if (pStr == "Both")
                                    p = PriorityType::Both;
                
                                editorApp->m_NodeToConditionParams[nodeKey]->Priority = p;
                            }
    
                            EditorCondition econd(c["Name"].as<std::string>());
                            econd.ClassName = className;
                            econd.Params = editorApp->m_NodeToConditionParams[nodeKey].get();
                            newNode->Conditions.push_back(econd);
                        }
                    }
                
                if (type == NodeType::Action && n["Params"])
                {
                    std::string actionClassName = n["Name"].as<std::string>();
                    auto& actionMap = NodeRegistry::GetActionClassInfoMap();
                    if (actionMap.count(actionClassName))
                    {
                        editorApp->m_NodeToActionClassId[nodeKey] = actionClassName;
                        editorApp->m_NodeToParams[nodeKey] = actionMap[actionClassName].CreateParamsFn();
                        editorApp->m_NodeToParams[nodeKey]->Deserialize(n["Params"]);
                    }
                }
            }
        }
    }
    else
        editorApp->GetNodeEditorHelper().SpawnRootNode();
    
    editorApp->GetNodeEditorHelper().BuildNodes();
    if (btNode["EditorData"]["Links"])
    {
        auto& helper = editorApp->GetNodeEditorHelper();
        for (auto l : btNode["EditorData"]["Links"])
        {
            int startNodeOldID = l["StartNodeID"].as<int>();
            int endNodeOldID = l["EndNodeID"].as<int>();
            
            auto itA = idMap.find(startNodeOldID);
            auto itB = idMap.find(endNodeOldID);
            if (itA == idMap.end() || itB == idMap.end())
                continue;

            Node* startNode = helper.FindNode(itA->second);
            Node* endNode   = helper.FindNode(itB->second);

            if (!startNode || !endNode)
                continue;

            if (!startNode->Outputs.empty() && !endNode->Inputs.empty())
            {
                nodeEditor::PinId startPin = startNode->Outputs[0].ID;
                nodeEditor::PinId endPin   = endNode->Inputs[0].ID;
                helper.GetLinks().emplace_back(Link(helper.GetNextLinkId(), startPin, endPin));
            }

        }
    }
    m_Tree = editorApp->BuildBehaviorTree();
    m_Tree->SetName(treeName);
    return true;
}

bool BTSerializer::DeserializeEditorGraphOnly(const YAML::Node& data, NodeEditorApp* editorApp)
{
    if (!editorApp)
        return false;

    auto btNode = data["BehaviorTree"];
    if (!btNode)
        return false;
    
    editorApp->ClearNodeMappings();
    /*editorApp->ClearActiveNodes();*/
    editorApp->GetNodeEditorHelper().ClearDatas();

    std::unordered_map<uintptr_t, uint64_t> editorIdToRuntimeUID;
    std::unordered_map<int, nodeEditor::NodeId> idMap;
    int maxID = 0;
    
    if (btNode["EditorData"] && btNode["EditorData"]["Nodes"])
    {
        for (auto node : btNode["EditorData"]["Nodes"])
        {
            int oldID = node["ID"].as<int>();
            maxID = std::max(maxID, oldID);

            std::string name = node["Name"].as<std::string>();
            NodeType type = static_cast<NodeType>(node["Type"].as<int>());
            ImVec2 pos(node["PosX"].as<float>(), node["PosY"].as<float>());

            Node* newNode = nullptr;
            auto& helper = editorApp->GetNodeEditorHelper();

            switch (type)
            {
                case NodeType::Root:
                    newNode = helper.SpawnRootNode();
                break;
                case NodeType::Sequence:
                    newNode = helper.SpawnSequenceNode(pos);
                    break;
                case NodeType::Selector:
                    newNode = helper.SpawnSelectorNode(pos);
                    break;
                case NodeType::Action:
                    newNode = helper.SpawnActionNode(pos);
                    break;
                default:
                    break;
            }
        
            if (!newNode)
                continue;
            
            uint64_t runtimeUID = 0;
            if (node["RuntimeUID"])
                runtimeUID = node["RuntimeUID"].as<uint64_t>();

            editorIdToRuntimeUID[(uintptr_t)newNode->ID.Get()] = runtimeUID;
            
            newNode->Name = name;
            nodeEditor::SetNodePosition(newNode->ID, pos);
            idMap[oldID] = newNode->ID;

            int nodeKey = (int)newNode->ID.Get();
            
            if (node["Decorators"])
                for (auto d : node["Decorators"])
                {
                    std::string className = d["ClassName"].as<std::string>();
                    auto& decoMap = NodeRegistry::GetDecoratorClassInfoMap();
                    if (!decoMap.count(className))
                        continue;

                    editorApp->m_NodeToDecoratorClassId[nodeKey] = className;
                    editorApp->m_NodeToDecoratorParams[nodeKey] = decoMap[className].CreateParamsFn();
                    editorApp->m_NodeToDecoratorParams[nodeKey]->Deserialize(d["Params"]);

                    EditorDecorator editorDeco(d["Name"].as<std::string>());
                    editorDeco.ClassName = className;
                    editorDeco.Params = editorApp->m_NodeToDecoratorParams[nodeKey].get();
                    newNode->Decorators.push_back(editorDeco);
                }
            if (node["Conditions"])
                for (auto c : node["Conditions"])
                {
                    std::string className = c["ClassName"].as<std::string>();
                    auto& condMap = NodeRegistry::GetConditionClassInfoMap();
                    if (!condMap.count(className))
                        continue;

                    editorApp->m_NodeToConditionClassId[nodeKey] = className;
                    editorApp->m_NodeToConditionParams[nodeKey] = condMap[className].CreateParamsFn();
                    editorApp->m_NodeToConditionParams[nodeKey]->Deserialize(c["Params"]);

                    if (c["Params"] && c["Params"]["Priority"])
                    {
                        std::string priorityStr = c["Params"]["Priority"].as<std::string>();
                        PriorityType priortyType = PriorityType::None;
                        if (priorityStr == "Self")
                            priortyType = PriorityType::Self;
                        else if (priorityStr == "LowerPriority")
                            priortyType = PriorityType::LowerPriority;
                        else if (priorityStr == "Both")
                            priortyType = PriorityType::Both;
                        editorApp->m_NodeToConditionParams[nodeKey]->Priority = priortyType;
                    }

                    EditorCondition econd(c["Name"].as<std::string>());
                    econd.ClassName = className;
                    econd.Params = editorApp->m_NodeToConditionParams[nodeKey].get();
                    newNode->Conditions.push_back(econd);
                }
            if (type == NodeType::Action && node["Params"])
            {
                std::string actionClassName = node["Name"].as<std::string>();
                auto& actionMap = NodeRegistry::GetActionClassInfoMap();
                if (actionMap.count(actionClassName))
                {
                    editorApp->m_NodeToActionClassId[nodeKey] = actionClassName;
                    editorApp->m_NodeToParams[nodeKey] = actionMap[actionClassName].CreateParamsFn();
                    editorApp->m_NodeToParams[nodeKey]->Deserialize(node["Params"]);
                }
            }
        }
    }
    else
        editorApp->GetNodeEditorHelper().SpawnRootNode();
    
    editorApp->GetNodeEditorHelper().BuildNodes();
    
    if (btNode["EditorData"] && btNode["EditorData"]["Links"])
    {
        auto& helper = editorApp->GetNodeEditorHelper();
        for (auto links : btNode["EditorData"]["Links"])
        {
            int startOld = links["StartNodeID"].as<int>();
            int endOld = links["EndNodeID"].as<int>();

            auto itA = idMap.find(startOld);
            auto itB = idMap.find(endOld);
            if (itA == idMap.end() || itB == idMap.end())
                continue;

            Node* startNode = helper.FindNode(itA->second);
            Node* endNode = helper.FindNode(itB->second);
            if (!startNode || !endNode)
                continue;

            if (!startNode->Outputs.empty() && !endNode->Inputs.empty())
            {
                nodeEditor::PinId sp = startNode->Outputs[0].ID;
                nodeEditor::PinId ep = endNode->Inputs[0].ID;
                helper.GetLinks().emplace_back(Link(helper.GetNextLinkId(), sp, ep));
            }
        }
    }
    
    BehaviorTree* runtimeTree = editorApp->m_BehaviorTree;
    if (runtimeTree && runtimeTree->GetRootNode())
    {
        std::unordered_map<uint64_t, const HNode*> uidToRuntime;
        CollectRuntimeNodesByUID(runtimeTree->GetRootNode(), uidToRuntime);
        
        auto& helper = editorApp->GetNodeEditorHelper();

        for (auto node : btNode["EditorData"]["Nodes"])
        {
            int oldID = node["ID"].as<int>();

            auto it = idMap.find(oldID);
            if (it == idMap.end())
                continue;

            Node* edNode = helper.FindNode(it->second);
            if (!edNode)
                continue;

            uint64_t uid = 0;
            if (node["RuntimeUID"])
                uid = node["RuntimeUID"].as<uint64_t>();

            if (uid == 0)
                continue;

            auto rtIt = uidToRuntime.find(uid);
            if (rtIt == uidToRuntime.end())
                continue;

            editorApp->RegisterNodeMapping(rtIt->second, edNode->ID);
        }
    }
    return true;
}

void BTSerializer::CollectRuntimeNodesByUID(const HNode* node, std::unordered_map<uint64_t, const HNode*>& out)
{
    if (!node)
        return;
    out[node->GetID()] = node;
    
    for (auto* c : node->GetChildrensRaw())
        CollectRuntimeNodesByUID(c, out);

    for (auto* cond : node->GetConditionNodesRaw())
        CollectRuntimeNodesByUID(cond, out);
}


const char* BTSerializer::NodeTypeToString(HNodeType type)
{
    switch (type)
    {
        case HNodeType::Root:
            return "Root";
        case HNodeType::Composite:
            return "Composite";
        case HNodeType::Action:
            return "Action";
        case HNodeType::Condition:
            return "Condition";
        case HNodeType::Decorator:
            return "Decorator";
        default:
            return "Unknown";
    }
}

const char* BTSerializer::PriorityToString(PriorityType p)
{
    switch (p)
    {
        case PriorityType::None:
            return "None";
        case PriorityType::Self:
            return "Self";
        case PriorityType::LowerPriority:
            return "LowerPriority";
        case PriorityType::Both:
            return "Both";
        default:
            return "None";
    }
}

void BTSerializer::SerializeBlackboard(YAML::Emitter& out, const HBlackboard* blackboard)
{
    if (!blackboard)
    {
        out << YAML::Key << "ClassName" << YAML::Value << "NoBlackboard";
        out << YAML::Key << "Floats" << YAML::Value << YAML::BeginMap << YAML::EndMap;
        out << YAML::Key << "Ints" << YAML::Value << YAML::BeginMap << YAML::EndMap;
        out << YAML::Key << "Bools" << YAML::Value << YAML::BeginMap << YAML::EndMap;
        out << YAML::Key << "Strings" << YAML::Value << YAML::BeginMap << YAML::EndMap;
        return;
    }
    auto classInfo = NodeRegistry::GetBlackboardClassInfoMap().find(blackboard->GetName());
    if (classInfo != NodeRegistry::GetBlackboardClassInfoMap().end())
        out << YAML::Key << "ClassName" << YAML::Value << classInfo->second.Name;
    
    out << YAML::Key << "Floats" << YAML::Value << YAML::BeginMap;
    for (const auto& [key, val] : blackboard->GetFloatValues())
        out << YAML::Key << key << YAML::Value << val;
    out << YAML::EndMap;
    
    out << YAML::Key << "Ints" << YAML::Value << YAML::BeginMap;
    for (const auto& [key, val] : blackboard->GetIntValues())
        out << YAML::Key << key << YAML::Value << val;
    out << YAML::EndMap;
    
    out << YAML::Key << "Bools" << YAML::Value << YAML::BeginMap;
    for (const auto& [key, val] : blackboard->GetBoolValues())
        out << YAML::Key << key << YAML::Value << val;
    out << YAML::EndMap;
    
    out << YAML::Key << "Strings" << YAML::Value << YAML::BeginMap;
    for (const auto& [key, val] : blackboard->GetStringValues())
        out << YAML::Key << key << YAML::Value << val;
    out << YAML::EndMap;
}

void BTSerializer::DeserializeBlackboard(const YAML::Node& blackboardNode, std::unique_ptr<HBlackboard>& blackboard)
{
    if (!blackboardNode || !blackboard)
        return;
    
    if (blackboardNode["Floats"])
        for (auto it = blackboardNode["Floats"].begin(); it != blackboardNode["Floats"].end(); ++it)
        {
            const std::string key = it->first.as<std::string>();
            const float value = it->second.as<float>();

            if (blackboard->HasFloatValue(key))
                blackboard->SetFloatValue(key, value);
            else
                blackboard->CreateFloatValue(key, value);
        }

    if (blackboardNode["Ints"])
        for (auto it = blackboardNode["Ints"].begin(); it != blackboardNode["Ints"].end(); ++it)
        {
            const std::string key = it->first.as<std::string>();
            const int value = it->second.as<int>();

            if (blackboard->HasIntValue(key))
                blackboard->SetIntValue(key, value);
            else
                blackboard->CreateIntValue(key, value);
        }

    if (blackboardNode["Bools"])
        for (auto it = blackboardNode["Bools"].begin(); it != blackboardNode["Bools"].end(); ++it)
        {
            const std::string key = it->first.as<std::string>();
            const bool value = it->second.as<bool>();

            if (blackboard->HasBoolValue(key))
                blackboard->SetBoolValue(key, value);
            else
                blackboard->CreateBoolValue(key, value);
        }

    if (blackboardNode["Strings"])
        for (auto it = blackboardNode["Strings"].begin(); it != blackboardNode["Strings"].end(); ++it)
        {
            const std::string key = it->first.as<std::string>();
            const std::string value = it->second.as<std::string>();

            if (blackboard->HasStringValue(key))
                blackboard->SetStringValue(key, value);
            else
                blackboard->CreateStringValue(key, value);
        }
}

void BTSerializer::DeserializeBlackboard(const YAML::Node& blackboardNode, HBlackboard* blackboard)
{
    if (!blackboardNode || !blackboard)
        return;
    
    if (blackboardNode["Floats"])
        for (auto it = blackboardNode["Floats"].begin(); it != blackboardNode["Floats"].end(); ++it)
        {
            const std::string key = it->first.as<std::string>();
            const float value = it->second.as<float>();

            if (blackboard->HasFloatValue(key))
                blackboard->SetFloatValue(key, value);
            else
                blackboard->CreateFloatValue(key, value);
        }

    if (blackboardNode["Ints"])
        for (auto it = blackboardNode["Ints"].begin(); it != blackboardNode["Ints"].end(); ++it)
        {
            const std::string key = it->first.as<std::string>();
            const int value = it->second.as<int>();

            if (blackboard->HasIntValue(key))
                blackboard->SetIntValue(key, value);
            else
                blackboard->CreateIntValue(key, value);
        }

    if (blackboardNode["Bools"])
        for (auto it = blackboardNode["Bools"].begin(); it != blackboardNode["Bools"].end(); ++it)
        {
            const std::string key = it->first.as<std::string>();
            const bool value = it->second.as<bool>();

            if (blackboard->HasBoolValue(key))
                blackboard->SetBoolValue(key, value);
            else
                blackboard->CreateBoolValue(key, value);
        }

    if (blackboardNode["Strings"])
        for (auto it = blackboardNode["Strings"].begin(); it != blackboardNode["Strings"].end(); ++it)
        {
            const std::string key = it->first.as<std::string>();
            const std::string value = it->second.as<std::string>();

            if (blackboard->HasStringValue(key))
                blackboard->SetStringValue(key, value);
            else
                blackboard->CreateStringValue(key, value);
        }
}

void BTSerializer::SerializeEditorData(YAML::Emitter& out)
{
    auto editorApp = EditorRoot::GetNodeEditorApp();
    if (!editorApp)
        return;

    auto& helper = editorApp->GetNodeEditorHelper();
    
    out << YAML::Key << "Nodes" << YAML::Value << YAML::BeginSeq;
    if (helper.GetNodes().empty())
        helper.SpawnRootNode();
    
    for (const auto& node : helper.GetNodes())
    {
        out << YAML::BeginMap;
        out << YAML::Key << "ID" << YAML::Value << node.ID.Get();
        out << YAML::Key << "Name" << YAML::Value << node.Name;
        out << YAML::Key << "Type" << YAML::Value << static_cast<int>(node.Type);
        auto runtimeNode = editorApp->GetRuntimeNodeFor(node.ID);
        if (runtimeNode)
        {
            out << YAML::Key << "RuntimeUID" << YAML::Value << runtimeNode->GetID();
            out << YAML::Key << "Class" << YAML::Value << typeid(*runtimeNode).name();
        }
        else
            out << YAML::Key << "RuntimeUID" << YAML::Value << (uint64_t)0;
        
        ImVec2 pos = nodeEditor::GetNodePosition(node.ID);
        out << YAML::Key << "PosX" << YAML::Value << pos.x;
        out << YAML::Key << "PosY" << YAML::Value << pos.y;
        
        out << YAML::Key << "Decorators" << YAML::Value << YAML::BeginSeq;
        for(const auto& deco : node.Decorators)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Name" << YAML::Value << deco.Name;
            out << YAML::Key << "ClassName" << YAML::Value << deco.ClassName;
            out << YAML::Key << "Params" << YAML::Value << YAML::BeginMap;
            if (deco.Params)
                deco.Params->Serialize(out);
            out << YAML::EndMap;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;

        out << YAML::Key << "Conditions" << YAML::Value << YAML::BeginSeq;
        for(const auto& cond : node.Conditions)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Name" << YAML::Value << cond.Name;
            out << YAML::Key << "ClassName" << YAML::Value << cond.ClassName;
            out << YAML::Key << "Params" << YAML::Value << YAML::BeginMap;
            out << YAML::Key << "Priority" << YAML::Value << PriorityToString(cond.Params->Priority);
            if (cond.Params)
                cond.Params->Serialize(out);
            out << YAML::EndMap;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        
        out << YAML::Key << "Params" << YAML::Value;
        out << YAML::BeginMap;
        if (runtimeNode)
            runtimeNode->GetParams().Serialize(out);
        out << YAML::EndMap;

        out << YAML::EndMap;
    }
    out << YAML::EndSeq;
    
    out << YAML::Key << "Links" << YAML::Value << YAML::BeginSeq;
    for (const auto& link : helper.GetLinks())
    {
        out << YAML::BeginMap;
        out << YAML::Key << "ID" << YAML::Value << link.ID.Get();
        
        auto* startPin = helper.FindPin(link.StartPinID);
        auto* endPin = helper.FindPin(link.EndPinID);
        
        if (startPin && endPin)
        {
            out << YAML::Key << "StartNodeID" << YAML::Value << startPin->Node->ID.Get();
            out << YAML::Key << "EndNodeID" << YAML::Value << endPin->Node->ID.Get();
        }
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;
}


void BTSerializer::SerializeChildren(YAML::Emitter& out, const HNode* node)
{
    auto children = node->GetChildrensRaw();

    out << YAML::Key << "Children" << YAML::Value;
    out << YAML::BeginSeq;

    for (const auto* child : children)
    {
        SerializeNode(out, child);
    }

    out << YAML::EndSeq;
}

void BTSerializer::SerializeNode(YAML::Emitter& out, const HNode* node)
{
    if (!node)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "NullNode" << YAML::Value << true;
        out << YAML::EndMap;
        return;
    }

    out << YAML::BeginMap;
    
    out << YAML::Key << "Name"  << YAML::Value << node->GetName();
    out << YAML::Key << "Type"  << YAML::Value << NodeTypeToString(node->GetType());
    
    out << YAML::Key << "Class" << YAML::Value << typeid(*node).name();
    SerializeConditions(out, node);

    if (node->GetType() == HNodeType::Decorator)
    {
        auto children = node->GetChildrensRaw();

        out << YAML::Key << "Child" << YAML::Value;

        if (!children.empty() && children[0])
            SerializeNode(out, children[0]);
        else
        {
            out << YAML::BeginMap;
            out << YAML::Key << "NullNode" << YAML::Value << true;
            out << YAML::EndMap;
        }
    }
    else if (node->GetType() == HNodeType::Root || node->GetType() == HNodeType::Composite)
    {
        SerializeChildren(out, node);
    }

    if (node->GetType() == HNodeType::Condition)
    {
        auto* cond = dynamic_cast<const HCondition*>(node);
        if (cond)
            out << YAML::Key << "Priority" << YAML::Value << PriorityToString(cond->GetPriorityMode());
    }

    out << YAML::Key << "Params" << YAML::Value;
    out << YAML::BeginMap;
    node->GetParams().Serialize(out);
    out << YAML::EndMap;

    out << YAML::EndMap;
}

void BTSerializer::DeserializeNodeRecursive(const YAML::Node& nodeData, BehaviorTreeBuilder& builder)
{
    if (!nodeData || nodeData["NullNode"])
        return;

    std::string name = nodeData["Name"].as<std::string>();
    std::string type = nodeData["Type"].as<std::string>();
    std::string className = nodeData["Class"].as<std::string>();

    if (type == "Root")
    {
        builder.root();
        if (nodeData["Children"]) 
            for (auto child : nodeData["Children"])
                DeserializeNodeRecursive(child, builder);
    }
    else if (type == "Decorator")
    {
        auto& decoMap = NodeRegistry::GetDecoratorClassInfoMap();
        auto it = decoMap.find(name);
        
        if (it != decoMap.end())
        {
            const YAML::Node& paramsNode = nodeData["Params"];
            it->second.BuildFromYAML(builder, name, paramsNode);
        }
        
        if (nodeData["Child"])
            DeserializeNodeRecursive(nodeData["Child"], builder);
    }
    else if (type == "Composite")
    {
        if (className.find("Selector") != std::string::npos)
            builder.selector(name);
        else
            builder.sequence(name);
        if (nodeData["Children"])
            for (auto child : nodeData["Children"])
                DeserializeNodeRecursive(child, builder);
        builder.end();
    }
    else if (type == "Action")
    {
        auto& actionMap = NodeRegistry::GetActionClassInfoMap();
        auto it = actionMap.find(name);
        if (it != actionMap.end())
        {
            const ActionClassInfo& actionInfo = it->second;
            if (actionInfo.BuildFromYAML)
            {
                const YAML::Node& paramsNode = nodeData["Params"];
                actionInfo.BuildFromYAML(builder, name, paramsNode);
            }
        }
    }

    if (nodeData["Conditions"])
        for (auto condData : nodeData["Conditions"])
            DeserializeCondition(condData, builder);
}

void BTSerializer::SerializeConditions(YAML::Emitter& out, const HNode* node)
{
    auto conditions = node->GetConditionNodesRaw();

    if (conditions.empty())
        return;

    out << YAML::Key << "Conditions" << YAML::Value;
    out << YAML::BeginSeq;

    for (const auto* cond : conditions)
        SerializeNode(out, cond);

    out << YAML::EndSeq;
}

void BTSerializer::DeserializeCondition(const YAML::Node& condData, BehaviorTreeBuilder& builder)
{
    std::string name = condData["Name"].as<std::string>();
    auto& condMap = NodeRegistry::GetConditionClassInfoMap();
    auto it = condMap.find(name);

    if (it != condMap.end())
    {
        PriorityType priority = PriorityType::None;
        if (condData["Priority"])
        {
            std::string pStr = condData["Priority"].as<std::string>();
            if (pStr == "Self")
                priority = PriorityType::Self;
            else if (pStr == "LowerPriority")
                priority = PriorityType::LowerPriority;
            else if (pStr == "Both")
                priority = PriorityType::Both;
        }

        const YAML::Node& paramsNode = condData["Params"];
        it->second.BuildFromYAML(builder, name, paramsNode, priority);
    }
}
