#include "NodeEditorApp.h"
#include <iostream>
#include "NodeEditorHelper.h"
#include "Tree.h"
#include "CustomThings/CustomActions.h"
#include "BTSerializer.h"
#include "NodeRegistry.h"
#include "PlatformUtilsBT.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <filesystem>

#include "imgui.h"

void NodeEditorApp::ClearDatas()
{
    m_bIsRuntimeMode = false;
    m_CopyBlackboard = nullptr;
    m_LastSelectedDecorator = nullptr;
    m_LastSelectedCondition = nullptr;
    m_LastHoveredNode = nullptr;
    m_LastSelectedNode = nullptr;
    m_SelectedBlackboardClassName.clear();

    ClearBuildData();
    if (m_NodeEditor)
        m_NodeEditor->ClearDatas();
    
    ClearNodeMappings();
}

NodeEditorApp::NodeEditorApp()
{
    m_NodeEditor = std::make_unique<NodeEditorHelper>(this);
}

NodeEditorApp::~NodeEditorApp()
{
    ClearDatas();
}

void NodeEditorApp::OnStart()
{
    m_NodeEditor->OnStart();
}

void NodeEditorApp::Update()
{
    DrawToolbar();
    
    DrawGraph();
    DrawBlackboard();
}

void NodeEditorApp::RegisterNodeMapping(const HNode* runtimeNode, nodeEditor::NodeId editorId)
{
    if (!runtimeNode)
        return;
    m_NodeToEditorIdMap[runtimeNode] = editorId;
    m_EditorIdToNodeMap[(uintptr_t)editorId.Get()] = runtimeNode;
}

Node* NodeEditorApp::GetEditorNodeFor(const HNode* runtimeNode)
{
    if (!runtimeNode)
        return nullptr;

    auto it = m_NodeToEditorIdMap.find(runtimeNode);
    if (it == m_NodeToEditorIdMap.end())
        return nullptr;

    return m_NodeEditor->FindNode(it->second);
}

const HNode* NodeEditorApp::GetRuntimeNodeFor(nodeEditor::NodeId editorId)
{
    if (editorId == nodeEditor::NodeId::Invalid)
        return nullptr;
    auto it = m_EditorIdToNodeMap.find((uintptr_t)editorId.Get());
    if (it == m_EditorIdToNodeMap.end())
        return nullptr;
    return it->second;
}

void NodeEditorApp::DecoratorNodeSelected(EditorDecorator& decorator)
{
    m_bDecoratorSelected = true; m_bConditionSelected = false;
    m_LastSelectedDecorator = &decorator;
}

void NodeEditorApp::ConditionNodeSelected(EditorCondition& condition)
{
    m_bConditionSelected = true; m_bDecoratorSelected = false;
    m_LastSelectedCondition = &condition;
}

void NodeEditorApp::DecoratorNodeUnSelected()
{
    m_bDecoratorSelected = false;
    m_LastSelectedDecorator = nullptr;
}

void NodeEditorApp::ConditionNodeUnSelected()
{
    m_bConditionSelected = false;
    m_LastSelectedCondition = nullptr;
}

void NodeEditorApp::MouseInputHandling()
{
    ImGuiIO& io = ImGui::GetIO();
    m_LastSelectedNode = m_NodeEditor->GetSelectedNode();
    auto lastHoveredNodeID = nodeEditor::GetHoveredNode();
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !io.KeyAlt)
    {
        bool isNodeHovered = nodeEditor::GetHoveredNode() || nodeEditor::GetHoveredPin() || nodeEditor::GetHoveredLink();
        if (!isNodeHovered)
        {
            m_LastHoveredNode = nullptr;
            ImGui::OpenPopup("Node Context");
        }
        else
        {
            m_LastHoveredNode = m_NodeEditor->FindNode(lastHoveredNodeID);
            if (m_LastHoveredNode->Type == NodeType::Sequence || m_LastHoveredNode->Type == NodeType::Selector || m_LastHoveredNode->Type == NodeType::Action)
                ImGui::OpenPopup("Node Options");
        }
    }
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !io.KeyAlt)
    {
        bool isNodeHovered = nodeEditor::GetHoveredNode() || nodeEditor::GetHoveredPin() || nodeEditor::GetHoveredLink();
        if (isNodeHovered)
        {
            m_LastHoveredNode = m_NodeEditor->FindNode(lastHoveredNodeID);
            if (m_LastHoveredNode->Type == NodeType::Sequence || m_LastHoveredNode->Type == NodeType::Selector || m_LastHoveredNode->Type == NodeType::Action)
            {
                m_bConditionSelected = false;
                m_bDecoratorSelected = false;
            }
        }
    }
}

void NodeEditorApp::NodeSettingsPanel()
{
    if (ImGui::BeginPopup("Node Context"))
    {
        ImGui::TextUnformatted("Node Context Menu");
        ImGui::Separator();

        ImVec2 mouseScreenPos = ImGui::GetMousePosOnOpeningCurrentPopup();
        ImVec2 mouseCanvasPos = nodeEditor::ScreenToCanvas(mouseScreenPos);
        if (ImGui::MenuItem("Create Sequence"))
            m_NodeEditor->SpawnSequenceNode(mouseCanvasPos);
        if (ImGui::MenuItem("Create Selector"))
            m_NodeEditor->SpawnSelectorNode(mouseCanvasPos);
        if (ImGui::MenuItem("Create Action"))
            m_NodeEditor->SpawnActionNode(mouseCanvasPos);
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopup("Node Options"))
    {
        ImGui::TextUnformatted("Node Options Menu");
        ImGui::Separator();
        if (ImGui::MenuItem("Create Decorator"))
            m_NodeEditor->SpawnDecoratorNode(m_LastHoveredNode);
        if (ImGui::MenuItem("Create Condition"))
            m_NodeEditor->SpawnConditionNode(m_LastHoveredNode);
        ImGui::EndPopup();
    }
}

void NodeEditorApp::FlowLinks()
{
    if (!m_BehaviorTree)
        return;
    
    auto activeNodes = m_BehaviorTree->GetActiveNodes();
    if (activeNodes.empty())
        m_NodeEditor->SetActiveNode(nullptr);
    for (int i = 0; i + 1 < (int)activeNodes.size(); ++i)
    {
        HNode* activeNode = activeNodes[i];
        HNode* nextNode = activeNodes[i + 1];

        if (!activeNode || !nextNode)
            continue;
        
        bool isParentChild = false;
        if (nextNode->GetParent() == activeNode)
            isParentChild = true;
        else
        {
            HNode* directParent = nextNode->GetParent();
            if (directParent)
                if (dynamic_cast<HDecorator*>(directParent) && directParent->GetParent() == activeNode)
                    isParentChild = true;
        }

        if (!isParentChild)
            continue;

        if (activeNode->GetStatus() != NodeStatus::RUNNING ||
            nextNode->GetStatus() != NodeStatus::RUNNING)
            continue;
        
        Node* activeEditorNode = GetEditorNodeFor(activeNode);
        Node* nextEditorNode = GetEditorNodeFor(nextNode);

        if (!activeEditorNode || !nextEditorNode)
            continue;

        if (activeEditorNode->Outputs.empty() || nextEditorNode->Inputs.empty())
            continue;

        auto outputPinID = activeEditorNode->Outputs[0].ID;
        auto inputPinID  = nextEditorNode->Inputs[0].ID;

        bool linkExists = false;
        auto linkID = nodeEditor::LinkId::Invalid;

        for (auto& link : m_NodeEditor->GetLinks())
            if (link.StartPinID == outputPinID && link.EndPinID == inputPinID)
            {
                linkID = link.ID;
                linkExists = true;
                break;
            }

        if (linkExists && linkID != nodeEditor::LinkId::Invalid)
            nodeEditor::Flow(linkID); //in imgui_node_editor in line 2964 i changed the duration for fade out
        auto lastActiveNode = activeNodes.back();
        auto lastEditorNode = GetEditorNodeFor(lastActiveNode);
        m_NodeEditor->SetActiveNode(lastEditorNode);
    }
}

void NodeEditorApp::BlackboardPanelSizeSettings()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos  = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;
    
    s_RightPanelWidth = std::clamp(s_RightPanelWidth, 200.0f, workSize.x * 0.8f);

    ImVec2 panelPos  = ImVec2(workPos.x + workSize.x - s_RightPanelWidth, workPos.y);
    ImVec2 panelSize = ImVec2(s_RightPanelWidth, workSize.y);

    ImGui::SetNextWindowPos(panelPos);
    ImGui::SetNextWindowSize(panelSize);
}

void NodeEditorApp::BlackboardPanel()
{
    if (m_bIsEmbedded)
    {
        DrawBlackboardContent();
        return;
    }
    
    BlackboardPanelSizeSettings();
    ImGui::Begin("Blackboard & Details", nullptr,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse);
    
    s_RightPanelWidth = ImGui::GetWindowSize().x;
    
    DrawBlackboardContent();

    ImGui::End();
}

void NodeEditorApp::ShowActionNodeInBlackboard()
{
    int nodeKey = (int)m_LastSelectedNode->ID.Get();

    ImGui::Text("Action Class");
    ImGui::Separator();

    std::string currentIdString;
    auto it = m_NodeToActionClassId.find(nodeKey);
    if (it != m_NodeToActionClassId.end())
        currentIdString = it->second;

    std::string currentLabel = "Select Action";
    if (!currentIdString.empty())
    {
        auto info = NodeRegistry::GetActionClassInfoMap().find(currentIdString);
        if (info != NodeRegistry::GetActionClassInfoMap().end())
            currentLabel = info->second.Name;
    }

    if (ImGui::BeginCombo("##ActionList", currentLabel.c_str()))
    {
        for (auto& [id, info] : NodeRegistry::GetActionClassInfoMap())
        {
            bool isSelected = (id == currentIdString);
            if (ImGui::Selectable(info.Name.c_str(), isSelected))
            {
                m_NodeToActionClassId[nodeKey] = id;
                m_NodeToParams[nodeKey] = info.CreateParamsFn();
                if (m_LastSelectedNode)
                    m_LastSelectedNode->Name = info.Name;
            }
        }
        ImGui::EndCombo();
    }

    DrawActionNodeParameters(nodeKey, currentIdString);
}

void NodeEditorApp::ShowDecoratorNodeInBlackboard()
{
    int nodeKey = (int)m_LastSelectedNode->ID.Get();

    ImGui::Text("Decorator Class");
    ImGui::Separator();

    std::string currentIdString;
    auto it = m_NodeToDecoratorClassId.find(nodeKey);
    if (it != m_NodeToDecoratorClassId.end())
        currentIdString = it->second;

    std::string currentLabel = "Select Decorator";
    if (!currentIdString.empty())
    {
        auto info = NodeRegistry::GetDecoratorClassInfoMap().find(currentIdString);
        if (info != NodeRegistry::GetDecoratorClassInfoMap().end())
            currentLabel = info->second.Name;
    }
    if (ImGui::BeginCombo("##DecoratorList", currentLabel.c_str()))
    {
        for (auto& [id, info] : NodeRegistry::GetDecoratorClassInfoMap())
        {
            bool isSelected = (id == currentIdString);
            if (ImGui::Selectable(info.Name.c_str(), isSelected))
            {
                m_NodeToDecoratorClassId[nodeKey] = id;
                m_NodeToDecoratorParams[nodeKey] = info.CreateParamsFn();
                if (m_LastSelectedDecorator)
                {
                    m_LastSelectedDecorator->Name = info.Name;
                    m_LastSelectedDecorator->ClassName = id;
                    m_LastSelectedDecorator->Params = m_NodeToDecoratorParams[nodeKey].get();
                }
            }
        }
        ImGui::EndCombo();
    }
    DrawDecoratorNodeParameters(nodeKey, currentIdString);
}

void NodeEditorApp::ShowConditionNodeInBlackboard()
{
    int nodeKey = (int)m_LastSelectedNode->ID.Get();

        ImGui::Text("Condition Class");
        ImGui::Separator();

        std::string currentIdString;
        auto it = m_NodeToConditionClassId.find(nodeKey);
        if (it != m_NodeToConditionClassId.end())
            currentIdString = it->second;

        std::string currentLabel = "Select Condition";
        if (!currentIdString.empty())
        {
            auto info = NodeRegistry::GetConditionClassInfoMap().find(currentIdString);
            if (info != NodeRegistry::GetConditionClassInfoMap().end())
                currentLabel = info->second.Name;
        }
        if (ImGui::BeginCombo("##ConditionList", currentLabel.c_str()))
        {
            for (auto& [id, info] : NodeRegistry::GetConditionClassInfoMap())
            {
                bool isSelected = (id == currentIdString);
                if (ImGui::Selectable(info.Name.c_str(), isSelected))
                {
                    m_NodeToConditionClassId[nodeKey] = id;
                    m_NodeToConditionParams[nodeKey] = info.CreateParamsFn();
                    if (m_LastSelectedCondition)
                    {
                        m_LastSelectedCondition->Name = info.Name;
                        m_LastSelectedCondition->ClassName = id;
                        m_LastSelectedCondition->Params = m_NodeToConditionParams[nodeKey].get();
                    }
                }
            }
            ImGui::EndCombo();
        }

    DrawConditionNodeParameters(nodeKey, currentIdString);
}

void NodeEditorApp::ShowBlackboardDetails()
{
    std::string currentLabel = "Select Blackboard";
    if (!m_SelectedBlackboardClassName.empty())
    {
        auto info = NodeRegistry::GetBlackboardClassInfoMap().find(m_SelectedBlackboardClassName);
        if (info != NodeRegistry::GetBlackboardClassInfoMap().end())
            currentLabel = info->second.Name;
    }
    if (ImGui::BeginCombo("##BlackboardOptions", currentLabel.c_str()))
    {
        for (auto& [id, info] : NodeRegistry::GetBlackboardClassInfoMap())
        {
            bool isSelected = (id == m_SelectedBlackboardClassName);
            if (ImGui::Selectable(info.Name.c_str(), isSelected))
                SetBlackboardForEditor(id, info);
        }
        ImGui::EndCombo();
    }
    if (!m_SelectedBlackboardClassName.empty())
    {
        if (m_CopyBlackboard)
            m_CopyBlackboard->DrawImGui();
        else
        {
            currentLabel = "Select Blackboard";
            m_SelectedBlackboardClassName.clear();
        }
    }
}

HBlackboard& NodeEditorApp::SetBlackboardForEditor(const std::string& id, const BlackboardClassInfo& info)
{
    m_SelectedBlackboardClassName = id;
    m_Blackboard = info.CreateBlackboardFn();
    m_CopyBlackboard = m_Blackboard.get();
    return *m_Blackboard;
}

void NodeEditorApp::DrawActionNodeParameters(int nodeKey, const std::string& classId)
{
    ImGui::Text("Parameters");
    ImGui::Separator();

    if (!classId.empty())
    {
        auto parameter = m_NodeToParams.find(nodeKey);
        if (parameter != m_NodeToParams.end() && parameter->second)
            parameter->second->DrawImGui(m_Blackboard.get());
    }
}

void NodeEditorApp::DrawDecoratorNodeParameters(int nodeKey, const std::string& classId)
{
    ImGui::Text("Parameters");
    ImGui::Separator();

    if (!classId.empty())
    {
        auto parameter = m_NodeToDecoratorParams.find(nodeKey);
        if (parameter != m_NodeToDecoratorParams.end() && parameter->second)
            parameter->second->DrawImGui(m_Blackboard.get());
    }
}

void NodeEditorApp::DrawConditionNodeParameters(int nodeKey, const std::string& classId)
{
    ImGui::Text("Parameters");
    ImGui::Separator();

    if (!classId.empty())
    {
        auto parameter = m_NodeToConditionParams.find(nodeKey);
        if (parameter != m_NodeToConditionParams.end() && parameter->second)
            parameter->second->DrawImGui(m_Blackboard.get());
    }
}

BehaviorTree* NodeEditorApp::BuildBehaviorTree()
{
    if (!m_Blackboard)
        for (auto& [id, info] : NodeRegistry::GetBlackboardClassInfoMap())
            if (id == m_SelectedBlackboardClassName)
            {
                m_SelectedBlackboardClassName = id;
                m_Blackboard = info.CreateBlackboardFn();
                if (m_Blackboard && m_CopyBlackboard && m_CopyBlackboard->GetName() == m_SelectedBlackboardClassName)
                {
                    m_Blackboard->SetBoolValues(m_CopyBlackboard->GetBoolValues());
                    m_Blackboard->SetIntValues(m_CopyBlackboard->GetIntValues());
                    m_Blackboard->SetFloatValues(m_CopyBlackboard->GetFloatValues());
                    m_Blackboard->SetStringValues(m_CopyBlackboard->GetStringValues());
                    m_CopyBlackboard = m_Blackboard.get();
                }
                break;
            }
    m_NodeEditor->BuildNodes();
    ClearBuildData();
    
    BehaviorTreeBuilder btBuilder;
    btBuilder.setBlackboard(std::move(m_Blackboard));
    
    btBuilder.root();
    if (auto* runtimeRoot = btBuilder.GetLastCreatedNode())
        for (auto& n : m_NodeEditor->GetNodes())
        {
            if (n.Type == NodeType::Root)
            {
                RegisterNodeMapping(runtimeRoot, n.ID);
                break;
            }
        }
    
    auto buildOps = CreateBuildPlan();
    for (BuildOp& op : buildOps)
        switch (op.Type)
        {
            case BuildOpType::OpenComposite:
            {
                Node* node = op.EditorNode;
                if (node->Type == NodeType::Sequence)
                    BuildSequence(node, btBuilder);
                else if (node->Type == NodeType::Selector)
                    BuildSelector(node, btBuilder);
                    
                if (auto* runtimeNode = btBuilder.GetLastCreatedNode())
                    RegisterNodeMapping(runtimeNode, node->ID);
                break;
            }
            case BuildOpType::Action:
            {    
                Node* node = op.EditorNode;  
                BuildAction(node, btBuilder);    
                if (auto* runtimeNode = btBuilder.GetLastCreatedNode())
                    RegisterNodeMapping(runtimeNode, node->ID);
                break;
            }
            case BuildOpType::CloseComposite:
            {
                btBuilder.end();
                break;
            }
            default:
                break;
        }
    m_BehaviorTree = btBuilder.build();
    m_BehaviorTree->SetNodeEditorApp(this);
    return m_BehaviorTree;
}

void NodeEditorApp::BuildSequence(Node* node, BehaviorTreeBuilder& btBuilder)
{
    int nodeKey = (int)node->ID.Get();
                    
    auto classIt = m_NodeToDecoratorClassId.find(nodeKey);
    if (classIt != m_NodeToDecoratorClassId.end())
    {
        const std::string& classId = classIt->second;

        auto infoIt = NodeRegistry::GetDecoratorClassInfoMap().find(classId);
        if (infoIt != NodeRegistry::GetDecoratorClassInfoMap().end())
        {
            DecoratorClassInfo& info = infoIt->second;

            auto paramsIt = m_NodeToDecoratorParams.find(nodeKey);
            if (paramsIt != m_NodeToDecoratorParams.end() && paramsIt->second)
            {
                ParamsForDecorator& decoParams = *paramsIt->second;
                info.BuildFn(btBuilder, decoParams);
            }
        }
    }
    btBuilder.sequence(node->Name);

    auto condClassIt = m_NodeToConditionClassId.find(nodeKey);
    if (condClassIt != m_NodeToConditionClassId.end())
    {
        const std::string& condClassId = condClassIt->second;

        auto condInfoIt = NodeRegistry::GetConditionClassInfoMap().find(condClassId);
        if (condInfoIt != NodeRegistry::GetConditionClassInfoMap().end())
        {
            ConditionClassInfo& condInfo = condInfoIt->second;

            auto condParamsIt = m_NodeToConditionParams.find(nodeKey);
            if (condParamsIt != m_NodeToConditionParams.end() && condParamsIt->second)
            {
                ParamsForCondition& condParams = *condParamsIt->second;
                condInfo.BuildFn(btBuilder, condParams);
            }
        }
    }
}

void NodeEditorApp::BuildSelector(Node* node, BehaviorTreeBuilder& btBuilder)
{
    int nodeKey = (int)node->ID.Get();

    auto classIt = m_NodeToDecoratorClassId.find(nodeKey);
    if (classIt != m_NodeToDecoratorClassId.end())
    {
        const std::string& classId = classIt->second;

        auto infoIt = NodeRegistry::GetDecoratorClassInfoMap().find(classId);
        if (infoIt != NodeRegistry::GetDecoratorClassInfoMap().end())
        {
            DecoratorClassInfo& info = infoIt->second;

            auto paramsIt = m_NodeToDecoratorParams.find(nodeKey);
            if (paramsIt != m_NodeToDecoratorParams.end() && paramsIt->second)
            {
                ParamsForDecorator& decoParams = *paramsIt->second;
                info.BuildFn(btBuilder, decoParams);
            }
        }
    }
    btBuilder.selector(node->Name);
    auto condClassIt = m_NodeToConditionClassId.find(nodeKey);
    if (condClassIt != m_NodeToConditionClassId.end())
    {
        const std::string& condClassId = condClassIt->second;

        auto condInfoIt = NodeRegistry::GetConditionClassInfoMap().find(condClassId);
        if (condInfoIt != NodeRegistry::GetConditionClassInfoMap().end())
        {
            ConditionClassInfo& condInfo = condInfoIt->second;

            auto condParamsIt = m_NodeToConditionParams.find(nodeKey);
            if (condParamsIt != m_NodeToConditionParams.end() && condParamsIt->second)
            {
                ParamsForCondition& condParams = *condParamsIt->second;
                condInfo.BuildFn(btBuilder, condParams);
            }
        }
    }
}

void NodeEditorApp::BuildAction(Node* node, BehaviorTreeBuilder& btBuilder)
{
    int nodeKey = (int)node->ID.Get();
    auto classIt = m_NodeToActionClassId.find(nodeKey);
    if (classIt == m_NodeToActionClassId.end())
        return;
    
    const std::string& classId = classIt->second;
    auto infoIt = NodeRegistry::GetActionClassInfoMap().find(classId);
    if (infoIt == NodeRegistry::GetActionClassInfoMap().end())
        return;
    
    ActionClassInfo& info = infoIt->second;
    auto paramsIt = m_NodeToParams.find(nodeKey);
    if (paramsIt == m_NodeToParams.end() || !paramsIt->second)
        return;
    
    ParamsForAction& params = *paramsIt->second;
        
    auto decoClassIt = m_NodeToDecoratorClassId.find(nodeKey);
    if (decoClassIt != m_NodeToDecoratorClassId.end())
    {
        const std::string& decoClassId = decoClassIt->second;
    
        auto decoInfoIt = NodeRegistry::GetDecoratorClassInfoMap().find(decoClassId);
        if (decoInfoIt != NodeRegistry::GetDecoratorClassInfoMap().end())
        {
            DecoratorClassInfo& decoInfo = decoInfoIt->second;
    
            auto decoParamsIt = m_NodeToDecoratorParams.find(nodeKey);
            if (decoParamsIt != m_NodeToDecoratorParams.end() && decoParamsIt->second)
            {
                ParamsForDecorator& decoParams = *decoParamsIt->second;
                decoInfo.BuildFn(btBuilder, decoParams);
            }
        }
    }
    
    info.BuildFn(btBuilder, node, params);
    
    auto condClassIt = m_NodeToConditionClassId.find(nodeKey);
    if (condClassIt != m_NodeToConditionClassId.end())
    {
        const std::string& condClassId = condClassIt->second;
    
        auto condInfoIt = NodeRegistry::GetConditionClassInfoMap().find(condClassId);
        if (condInfoIt != NodeRegistry::GetConditionClassInfoMap().end())
        {
            ConditionClassInfo& condInfo = condInfoIt->second;
    
            auto condParamsIt = m_NodeToConditionParams.find(nodeKey);
            if (condParamsIt != m_NodeToConditionParams.end() && condParamsIt->second)
            {
                ParamsForCondition& condParams = *condParamsIt->second;
                condInfo.BuildFn(btBuilder, condParams);
            }
        }
    }
}

void NodeEditorApp::ClearBuildData()
{
    ClearNodeMappings();
    /*ClearActiveNodes();*/
    if (m_BehaviorTree)
        m_BehaviorTree->SetNodeEditorApp(nullptr);
    m_BehaviorTree = nullptr;
}

void NodeEditorApp::BuildPlanForNode(Node* editorNode, std::vector<BuildOp>& ops)
{
    switch (editorNode->Type)
    {
    case NodeType::Root:
        {
            auto children = m_NodeEditor->GetChilderenNodes(editorNode);
            if (!children.empty())
                BuildPlanForNode(children[0], ops);
            break;
        }
    case NodeType::Sequence:
    case NodeType::Selector:
        {
            ops.push_back({ BuildOpType::OpenComposite, editorNode });
            
            auto children = m_NodeEditor->GetChilderenNodes(editorNode);
            std::sort(children.begin(), children.end(),
                [](Node* a, Node* b)
                {
                    ImVec2 pa = nodeEditor::GetNodePosition(a->ID);
                    ImVec2 pb = nodeEditor::GetNodePosition(b->ID);
                    if (pa.x == pb.x)
                        return pa.y < pb.y;
                    return pa.x < pb.x;
                });         
            for (Node* child : children)
                BuildPlanForNode(child, ops);
            
            ops.push_back({ BuildOpType::CloseComposite, nullptr });
            break;
        }
    case NodeType::Action:
        {
            ops.push_back({ BuildOpType::Action, editorNode });
            break;
        }
    default:
        break;
    }
}

std::vector<BuildOp> NodeEditorApp::CreateBuildPlan()
{
    std::vector<BuildOp> ops;
    
    Node* rootEditorNode = nullptr;
    for (auto& n : m_NodeEditor->GetNodes())
        if (n.Type == NodeType::Root)
        {
            rootEditorNode = &n;
            break;
        }

    if (!rootEditorNode)
    {
        return ops;
    }

    BuildPlanForNode(rootEditorNode, ops);
    return ops;
}

void NodeEditorApp::DrawBlackboardContent()
{
    ImGui::TextUnformatted("Blackboard / Node Details Panel");
    ImGui::Separator();

    if (m_LastSelectedNode && m_LastSelectedNode->Type == NodeType::Action && !m_bDecoratorSelected && !m_bConditionSelected)
        ShowActionNodeInBlackboard();
    else if (m_LastSelectedNode && m_bDecoratorSelected)
        ShowDecoratorNodeInBlackboard();
    else if (m_LastSelectedNode && m_bConditionSelected)
        ShowConditionNodeInBlackboard();

    if (!m_LastSelectedNode)
        ShowBlackboardDetails();
}

void NodeEditorApp::DrawToolbar()
{
    if (ImGui::Button("Build", ImVec2(100, 30)))
        BuildBehaviorTree();
    ImGui::SameLine();
    /*if (ImGui::Button("Start", ImVec2(100, 30)))
    {
        if (m_BehaviorTree)
            m_BehaviorTree->StartTree();
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop", ImVec2(100, 30)))
    {
        if (m_BehaviorTree)
            m_BehaviorTree->StopTree();
        m_NodeEditor->SetActiveNode(nullptr);
    }*/
    if (IsRuntimeMode())
        DrawDebugBehaviorTree();
    ImGui::Separator();
    if (ImGui::Button("Save", ImVec2(150, 30)))
    {
        std::cout << "Save Button Clicked" << std::endl;
        if (!m_BehaviorTree)
        {
            return;
        }
        if (m_CurrentBTFilePath.empty())
        {
            std::string filePath = PlatformUtilsBT::SaveFile("Behavior Tree File (*.btree)\0*.btree\0");
            BTSerializer serializer(m_BehaviorTree);
            serializer.Serialize(filePath);
            m_CurrentBTFilePath = filePath;
            return;
        }
        BTSerializer serializer(m_BehaviorTree);
        serializer.Serialize(m_CurrentBTFilePath);
    }
    ImGui::SameLine();
    if (ImGui::Button("Load", ImVec2(150, 30)))
    {
        if (m_BehaviorTree)
            m_BehaviorTree->SetNodeEditorApp(nullptr);
        m_CopyBlackboard = nullptr;
        m_Blackboard = nullptr;
        ClearBuildData();
        std::string filePath = PlatformUtilsBT::OpenFile("Behavior Tree File (*.btree)\0*.btree\0");
        BTSerializer serializer(m_BehaviorTree);
        serializer.Deserialize(filePath, this);
        m_CurrentBTFilePath = filePath;
    }
    ImGui::SameLine();
    if (ImGui::Button("Save As", ImVec2(150, 30)))
    {
        if (!m_BehaviorTree)
            return;
        std::string filePath = PlatformUtilsBT::SaveFile("Behavior Tree File (*.btree)\0*.btree\0");
        BTSerializer serializer(m_BehaviorTree);
        serializer.Serialize(filePath);
        m_CurrentBTFilePath = filePath;
    }
}

void NodeEditorApp::DrawGraph()
{
    MouseInputHandling();
    NodeSettingsPanel();
    
    int linkAmount = (int)m_NodeEditor->GetLinks().size();
    ImGui::Text("Links: %d", linkAmount);

    FlowLinks();
    m_NodeEditor->OnUpdate();
}

void NodeEditorApp::DrawBlackboard()
{
    BlackboardPanel();
}

void NodeEditorApp::DrawDebugBehaviorTree()
{
    auto allTrees = Root::GetBehaviorTrees();
    const char* label = m_BehaviorTree ? m_BehaviorTree->GetName().c_str() : "Select Tree";

    ImGui::SetNextItemWidth(220.0f);
    if (ImGui::BeginCombo("##DebugTreeCombo", label))
    {
        for (auto& tree : allTrees)
        {
            if (ImGui::Selectable(tree->GetName().c_str(), m_BehaviorTree == tree))
                CreateEditorTreeFromRuntimeTree(tree);

            if (m_BehaviorTree == tree)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}


void NodeEditorApp::CreateEditorTreeFromRuntimeTree(BehaviorTree* runtimeTree)
{
    if (!runtimeTree)
        return;
    
    if (m_BehaviorTree)
        m_BehaviorTree->SetNodeEditorApp(nullptr);

    m_BehaviorTree = runtimeTree;
    m_BehaviorTree->SetNodeEditorApp(this);

    ClearNodeMappings();
    /*ClearActiveNodes();*/

    m_CopyBlackboard = m_BehaviorTree->GetBlackboardRaw();
    if (m_CopyBlackboard)
        m_SelectedBlackboardClassName = m_CopyBlackboard->GetName();

    const std::string path = Root::GetBehaviorTreePath(m_BehaviorTree);
    if (path.empty() || !std::filesystem::exists(path))
        return;

    YAML::Node data = YAML::LoadFile(path);
    
    BTSerializer::DeserializeEditorGraphOnly(data, this);
}
