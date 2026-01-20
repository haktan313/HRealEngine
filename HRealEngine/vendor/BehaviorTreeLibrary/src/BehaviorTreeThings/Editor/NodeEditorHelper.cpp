
#include "NodeEditorHelper.h"
#include <iostream>

#include "NodeEditorApp.h"

namespace nodeEditor = ax::NodeEditor;

void NodeEditorHelper::ClearDatas()
{
    m_Nodes.clear();
    m_Links.clear();
    m_NextId = 1;
    newLinkPin = nullptr;
    m_ActiveNode = nullptr;
}

NodeEditorHelper::NodeEditorHelper(NodeEditorApp* app) : m_App(app)
{
}

void NodeEditorHelper::OnStart()
{
    nodeEditor::Config config;
    config.UserPointer = this;

    /*config.LoadNodeSettings = [](nodeEditor::NodeId nodeId, char* data, void* userPointer) -> size_t
    {
        auto self = static_cast<NodeEditorHelper*>(userPointer);

        auto node = self->FindNode(nodeId);
        if (!node)
            return 0;

        if (data != nullptr)
            memcpy(data, node->State.data(), node->State.size());
        return node->State.size();
    };

    config.SaveNodeSettings = [](nodeEditor::NodeId nodeId, const char* data, size_t size, nodeEditor::SaveReasonFlags reason, void* userPointer) -> bool
    {
        auto self = static_cast<NodeEditorHelper*>(userPointer);

        auto node = self->FindNode(nodeId);
        if (!node)
            return false;

        node->State.assign(data, size);

        self->TouchNode(nodeId);

        return true;
    };*/
    
    m_EditorContext = nodeEditor::CreateEditor(&config);
    nodeEditor::SetCurrentEditor(m_EditorContext);
    nodeEditor::NavigateToContent();

    SpawnRootNode();
    
    BuildNodes();
}

void NodeEditorHelper::OnUpdate()
{
    ImGui::Separator();
    nodeEditor::SetCurrentEditor(m_EditorContext);
    nodeEditor::Begin("My nodeEditoritor", ImVec2(0.0, 0.0f));
    
    for (auto& node : m_Nodes)
    {
        const float padding  = 12.0f;
        const auto pinBackground = nodeEditor::GetStyle().Colors[nodeEditor::StyleColor_NodeBg];
        StylizeNodes();
        
        nodeEditor::BeginNode(node.ID);
        ImGui::BeginVertical(node.ID.AsPointer());
        ImGui::BeginHorizontal("inputs");
        ImGui::Spring(0, padding * 2);  
        
        ImRect inputsRect;
        int inputAlpha = 200;
        ManageInputs(inputsRect, inputAlpha, node, padding);
        
        ImGui::Spring(0, padding * 2);
        ImGui::EndHorizontal();
        
        ImGui::BeginHorizontal("content_frame");
        ImGui::Spring(1, padding);

        ImRect decoratorRect;
        ImRect conditionRect;
        ImRect sequenceRect;

        ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
        DrawDecoratorBar(node, decoratorRect);
        
        ImGui::Dummy(ImVec2(160, 0));
        ImGui::BeginHorizontal("sequence_row");
        ImGui::Spring(1);
        ImGui::TextUnformatted(node.Name.c_str());
        ImGui::Spring(1);
        ImGui::EndHorizontal();
        sequenceRect = nodeEditor::Detail::ImGui_GetItemRect();
        
        DrawConditionBar(node, conditionRect);
        ImGui::EndVertical();
        
        ImRect contentRect = sequenceRect;
        if (node.Decorators.empty())
            decoratorRect = ImRect(sequenceRect.Min, sequenceRect.Min);
        else
        { 
            contentRect.Add(decoratorRect.Min);
            contentRect.Add(decoratorRect.Max);
        }
        if (!node.Conditions.empty())
        {
            contentRect.Add(conditionRect.Min);
            contentRect.Add(conditionRect.Max);
        }

        ImGui::Spring(1, padding);
        ImGui::EndHorizontal();

        
        ImGui::BeginHorizontal("outputs");
        ImGui::Spring(0, padding * 2);
        ImRect outputsRect;
        int outputAlpha = 200;
        ManageOutputs(outputsRect, outputAlpha, node, padding);
        
        ImGui::Spring(0, padding * 2);
        ImGui::EndHorizontal(); 
        ImGui::EndVertical();   
        nodeEditor::EndNode();
        
        PaintNodeBackground(node, inputsRect, outputsRect, contentRect, pinBackground, inputAlpha, outputAlpha, sequenceRect);
    }
    ManageLinks();
    nodeEditor::End();
}

Node* NodeEditorHelper::SpawnRootNode()
{
    m_Nodes.emplace_back(NodeType::Root, GetNextID(), "Root");
    m_Nodes.back().Outputs.emplace_back(GetNextID(), "");
    nodeEditor::SetNodePosition(m_Nodes.back().ID, ImVec2(0, 0));
    BuildNode(&m_Nodes.back());
    m_RootOutputPinId = m_Nodes.back().Outputs[0].ID;
    return &m_Nodes.back();
}

Node* NodeEditorHelper::SpawnSequenceNode(ImVec2 position)
{
    m_Nodes.emplace_back(NodeType::Sequence, GetNextID(), "Sequence");
    m_Nodes.back().Inputs.emplace_back(GetNextID(), "");
    m_Nodes.back().Outputs.emplace_back(GetNextID(), "");
    nodeEditor::SetNodePosition(m_Nodes.back().ID, position);

    BuildNode(&m_Nodes.back());

    return &m_Nodes.back();
}

Node* NodeEditorHelper::SpawnSelectorNode(ImVec2 position)
{
    m_Nodes.emplace_back(NodeType::Selector, GetNextID(), "Selector");
    m_Nodes.back().Inputs.emplace_back(GetNextID(), "");
    m_Nodes.back().Outputs.emplace_back(GetNextID(), "");
    nodeEditor::SetNodePosition(m_Nodes.back().ID, position);
    BuildNode(&m_Nodes.back());
    return &m_Nodes.back();
}

Node* NodeEditorHelper::SpawnActionNode(ImVec2 position)
{
    m_Nodes.emplace_back(NodeType::Action, GetNextID(), "Action");
    m_Nodes.back().Inputs.emplace_back(GetNextID(), "");
    nodeEditor::SetNodePosition(m_Nodes.back().ID, position);
    BuildNode(&m_Nodes.back());
    return &m_Nodes.back();
}

void NodeEditorHelper::SpawnConditionNode(Node* parentNode)
{
    if (!parentNode)
        return;
    EditorCondition condition("Condition");
    parentNode->Conditions.push_back(condition);
}

void NodeEditorHelper::SpawnDecoratorNode(Node* parentNode)
{
    if (!parentNode)
        return;
    EditorDecorator decorator("Decorator");
    parentNode->Decorators.push_back(decorator);
}

std::vector<Node*> NodeEditorHelper::GetChilderenNodes(Node* parentNode)
{
    std::vector<Node*> childrenNodes;
    if (!parentNode || parentNode->Outputs.empty())
        return childrenNodes;
    nodeEditor::PinId parentOutputPinId = parentNode->Outputs[0].ID;
    for (auto& link : m_Links)
    {
        if (link.StartPinID != parentOutputPinId)
            continue;
        
        for (auto& node : m_Nodes)
        {
            bool found = false;

            for (auto& pin : node.Inputs)
            {
                if (pin.ID == link.EndPinID)
                {
                    childrenNodes.push_back(&node);
                    found = true;
                    break;
                }
            }

            if (found)
                break;
        }
    }
    return childrenNodes;
}

Node* NodeEditorHelper::GetSelectedNode()
{
    int count = nodeEditor::GetSelectedObjectCount();
    std::vector<nodeEditor::NodeId> selectedNodes(count);
    int realCount = nodeEditor::GetSelectedNodes(selectedNodes.data(), count);
    for (int i = 0; i < realCount; ++i)
    {
        Node* n = FindNode(selectedNodes[i]);
        if (n)
            return n;
    }
    return nullptr;
}

Node* NodeEditorHelper::FindNode(nodeEditor::NodeId id)
{
    for (auto& node : m_Nodes)
        if (node.ID == id)
            return &node;

    return nullptr;
}

Pin* NodeEditorHelper::FindPin(nodeEditor::PinId id)
{
    if (!id)
        return nullptr;

    for (auto& node : m_Nodes)
    {
        for (auto& pin : node.Inputs)
            if (pin.ID == id)
                return &pin;

        for (auto& pin : node.Outputs)
            if (pin.ID == id)
                return &pin;
    }

    return nullptr;
}

void NodeEditorHelper::BuildNodes()
{
    for (auto& node : m_Nodes)
        BuildNode(&node);
}

void NodeEditorHelper::BuildNode(Node* node)
{
    for (auto& input : node->Inputs)
    {
        input.Node = node;
        input.Kind = PinKind::Input;
    }

    for (auto& output : node->Outputs)
    {
        output.Node = node;
        output.Kind = PinKind::Output;
    }
}

void NodeEditorHelper::ManageInputs(ImRect& inputsRect, int& inputAlpha, Node& node, float padding)
{
    if (!node.Inputs.empty())
    {
        auto& pin = node.Inputs[0];
        ImGui::Dummy(ImVec2(0, padding));
        ImGui::Spring(1, 0);
        inputsRect = nodeEditor::Detail::ImGui_GetItemRect();   
            
        nodeEditor::PushStyleVar(nodeEditor::StyleVar_PinArrowSize, 10.0f);
        nodeEditor::PushStyleVar(nodeEditor::StyleVar_PinArrowWidth, 10.0f);
        nodeEditor::PushStyleVar(nodeEditor::StyleVar_PinCorners, ImDrawFlags_RoundCornersBottom);
            
        nodeEditor::BeginPin(pin.ID, nodeEditor::PinKind::Input);
        nodeEditor::PinPivotRect(inputsRect.GetTL(), inputsRect.GetBR());
        nodeEditor::PinRect(inputsRect.GetTL(), inputsRect.GetBR());
        nodeEditor::EndPin();
            
        nodeEditor::PopStyleVar(3); 
        if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
            inputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
    }
    else
        ImGui::Dummy(ImVec2(0, padding));   
}

void NodeEditorHelper::DrawDecoratorBar(Node& node, ImRect& decoratorRect)
{
    if (!node.Decorators.empty())
    {

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 2));
        ImGui::PushStyleColor(ImGuiCol_Button,        ImColor(24, 64, 128, 255).Value);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(40, 100, 200, 255).Value);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImColor(60, 150, 255, 255).Value);

        ImRect decoratorsUnion;
        bool hasDecoratorRect = false;
        float decoratorButtonWidth = 160.0f;

        for (int i = 0; i < static_cast<int>(node.Decorators.size()); ++i)
        {
            ImGui::BeginHorizontal(("decorator_row_" + std::to_string(i)).c_str());
            bool decoClicked = ImGui::Button(node.Decorators[i].Name.c_str(), ImVec2(decoratorButtonWidth, 0.0f));
            ImGui::EndHorizontal();
                
            ImRect thisRect = nodeEditor::Detail::ImGui_GetItemRect();
            if (!hasDecoratorRect)
            {
                decoratorsUnion = thisRect;
                hasDecoratorRect = true;
            }
            else
            {
                decoratorsUnion.Add(thisRect.Min);
                decoratorsUnion.Add(thisRect.Max);
            }
            if (decoClicked)
                m_App->DecoratorNodeSelected(node.Decorators[i]);
        }
        decoratorRect = decoratorsUnion;

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);
    }
}

void NodeEditorHelper::DrawConditionBar(Node& node, ImRect& conditionRect)
{
    if (!node.Conditions.empty())
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 2));
        ImGui::PushStyleColor(ImGuiCol_Button,        ImColor(120, 30, 30, 255).Value);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(160, 40, 40, 255).Value);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImColor(200, 50, 50, 255).Value);
        
        ImRect conditionsUnion;
        bool hasCondRect = false;
        float conditionButtonWidth = 160.0f;

        for (int i = 0; i < (int)node.Conditions.size(); ++i)
        {
            ImGui::BeginHorizontal(("condition_row_" + std::to_string(i)).c_str());
            bool condClicked = ImGui::Button(node.Conditions[i].Name.c_str(), ImVec2(conditionButtonWidth, 0.0f));
            ImGui::EndHorizontal();

            ImRect thisRect = nodeEditor::Detail::ImGui_GetItemRect();
            if (!hasCondRect)
            {
                conditionsUnion = thisRect;
                hasCondRect = true;
            }
            else
            {
                conditionsUnion.Add(thisRect.Min);
                conditionsUnion.Add(thisRect.Max);
            }

            if (condClicked)
                m_App->ConditionNodeSelected(node.Conditions[i]);
        }

        conditionRect = conditionsUnion;

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);
    }
}

void NodeEditorHelper::ManageOutputs(ImRect& outputsRect, int& outputAlpha, Node& node, float padding)
{
    if (!node.Outputs.empty())
    {
        auto& pin = node.Outputs[0];
        ImGui::Dummy(ImVec2(0, padding));
        ImGui::Spring(1, 0);
        outputsRect = ax::NodeEditor::Detail::ImGui_GetItemRect();
        
        nodeEditor::PushStyleVar(nodeEditor::StyleVar_PinCorners, ImDrawFlags_RoundCornersTop);
        
        nodeEditor::BeginPin(pin.ID, nodeEditor::PinKind::Output);
        nodeEditor::PinPivotRect(outputsRect.GetTL(), outputsRect.GetBR());
        nodeEditor::PinRect(outputsRect.GetTL(), outputsRect.GetBR());
        nodeEditor::EndPin();
        
        nodeEditor::PopStyleVar();  
        if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
            outputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
    }
    else
        ImGui::Dummy(ImVec2(0, padding));   
}

void NodeEditorHelper::ManageLinks()
{
    for (auto& link : m_Links)
        nodeEditor::Link(link.ID, link.StartPinID, link.EndPinID, link.Color, 2.0f);
    if (nodeEditor::BeginCreate(ImColor(255, 255, 255), 2.0f))
    {
        auto showLabel = [](const char* label, ImColor color)
        {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
            auto size = ImGui::CalcTextSize(label);
            
            auto padding = ImGui::GetStyle().FramePadding;
            auto spacing = ImGui::GetStyle().ItemSpacing;
            
            ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));
            
            auto rectMin = ImGui::GetCursorScreenPos() - padding;
            auto rectMax = ImGui::GetCursorScreenPos() + size + padding;
            
            auto drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
            ImGui::TextUnformatted(label);
        };  
        nodeEditor::PinId startPinId = 0, endPinId = 0;
        if (nodeEditor::QueryNewLink(&startPinId, &endPinId))
        {
            auto startPin = FindPin(startPinId);
            auto endPin = FindPin(endPinId);
            
            newLinkPin = startPin ? startPin : endPin;
            
            if (startPin && endPin)
            {
                if (startPin->Kind == PinKind::Input)
                {
                    std::swap(startPin, endPin);
                    std::swap(startPinId, endPinId);
                }   
                if (endPin == startPin)
                {
                    nodeEditor::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                }
                else if (endPin->Kind == startPin->Kind)
                {
                    showLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
                    nodeEditor::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                }
                else
                {
                    showLabel("+ Create Link", ImColor(32, 45, 32, 180));
                    if (nodeEditor::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
                    {
                        if (startPinId == m_RootOutputPinId)
                        {
                            auto rootOutputPinId = startPinId;
                            for (int i = 0; i < (int)m_Links.size(); ++i)
                                if (m_Links[i].StartPinID == rootOutputPinId)
                                {
                                    m_Links.erase(m_Links.begin() + i);
                                    break;
                                }
                        }
                        m_Links.emplace_back(Link(GetNextID(), startPinId, endPinId));
                        m_Links.back().Color = ImColor(255, 255, 255);
                    }
                }
            }
        }   
    }
    else
        newLinkPin = nullptr;
    
    nodeEditor::EndCreate();    
    if (nodeEditor::BeginDelete())
    {
        nodeEditor::NodeId nodeId = 0;
        while (nodeEditor::QueryDeletedNode(&nodeId))
        {
            if (nodeEditor::AcceptDeletedItem())
            {
                auto id = std::find_if(m_Nodes.begin(), m_Nodes.end(), [nodeId](auto& node) { return node.ID == nodeId; });
                if (id != m_Nodes.end())
                    m_Nodes.erase(id);
            }
        }   
        nodeEditor::LinkId linkId = 0;
        while (nodeEditor::QueryDeletedLink(&linkId))
        {
            if (nodeEditor::AcceptDeletedItem())
            {
                auto id = std::find_if(m_Links.begin(), m_Links.end(), [linkId](auto& link) { return link.ID == linkId; });
                if (id != m_Links.end())
                    m_Links.erase(id);
            }
        }
    }
    nodeEditor::EndDelete();
    
}

bool NodeEditorHelper::CanCreateLink(Pin* a, Pin* b)
{
    if (!a || !b || a == b || a->Kind == b->Kind || a->Node == b->Node)
        return false;
    return true;
}

void NodeEditorHelper::StylizeNodes()
{
    const float rounding = 5.0f;

    nodeEditor::PushStyleColor(nodeEditor::StyleColor_NodeBg, ImColor(128, 128, 128, 200)); // Background color
    nodeEditor::PushStyleColor(nodeEditor::StyleColor_NodeBorder, ImColor( 32,  32,  32, 200)); // Border color
    nodeEditor::PushStyleColor(nodeEditor::StyleColor_PinRect, ImColor( 60, 180, 255, 150)); // When pin is hovered
    nodeEditor::PushStyleColor(nodeEditor::StyleColor_PinRectBorder, ImColor( 60, 180, 255, 150)); 

    nodeEditor::PushStyleVar(nodeEditor::StyleVar_NodePadding, ImVec4(0, 0, 0, 0));
    nodeEditor::PushStyleVar(nodeEditor::StyleVar_NodeRounding, rounding); // Node rounding
    nodeEditor::PushStyleVar(nodeEditor::StyleVar_SourceDirection, ImVec2(0.0f,  1.0f));
    nodeEditor::PushStyleVar(nodeEditor::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
    nodeEditor::PushStyleVar(nodeEditor::StyleVar_LinkStrength, 0.0f);// 0.0f = linear, 1.0f = curved
    nodeEditor::PushStyleVar(nodeEditor::StyleVar_PinBorderWidth, 1.0f);// Pin border width
    nodeEditor::PushStyleVar(nodeEditor::StyleVar_PinRadius, 5.0f);
}

void NodeEditorHelper::PaintNodeBackground(Node& node, const ImRect& inputsRect, const ImRect& outputsRect, const ImRect& contentRect, const ImVec4& pinBackground,
    int inputAlpha, int outputAlpha, const ImRect& sequenceRect)
{
    nodeEditor::PopStyleVar(7);
    nodeEditor::PopStyleColor(4);   
    auto drawList = nodeEditor::GetNodeBackgroundDrawList(node.ID);
    const auto topRoundCornersFlags = ImDrawFlags_RoundCornersTop;
    const auto bottomRoundCornersFlags = ImDrawFlags_RoundCornersBottom;
        
    drawList->AddRectFilled(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
        IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, bottomRoundCornersFlags);
    //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
    drawList->AddRect(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
        IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, bottomRoundCornersFlags);
    //ImGui::PopStyleVar();
    drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
        IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, topRoundCornersFlags);
    //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
    drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
        IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, topRoundCornersFlags);
    //ImGui::PopStyleVar();
    drawList->AddRectFilled(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(24, 64, 128, 200), 0.0f);
    //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
    drawList->AddRect(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(48, 128, 255, 100), 0.0f);
    //ImGui::PopStyleVar();
    
    drawList->AddRectFilled(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(50, 50, 50, 230), 0.0f);
    drawList->AddRect( contentRect.GetTL(), contentRect.GetBR(), IM_COL32(20, 20, 20, 255), 0.0f);
    
    drawList->AddRectFilled( sequenceRect.GetTL(), sequenceRect.GetBR(), IM_COL32(30, 30, 30, 255), 0.0f);
    drawList->AddRect( sequenceRect.GetTL(), sequenceRect.GetBR(), IM_COL32(10, 10, 10, 255), 0.0f);

    if (&node == m_ActiveNode)
    {
        auto glowColor = IM_COL32(0, 255, 0, 200);
        auto borderColor = IM_COL32(0, 180, 0, 255);
        
        ImRect highlightRect = contentRect;
        highlightRect.Expand(6.0f);
        
        drawList->AddRect(
            highlightRect.Min,
            highlightRect.Max,
            glowColor,
            6.0f,
            ImDrawFlags_RoundCornersAll,
            4.0f
        );
        
        highlightRect.Expand(-2.0f);
        drawList->AddRect(
            highlightRect.Min,
            highlightRect.Max,
            borderColor,
            6.0f,
            ImDrawFlags_RoundCornersAll,
            2.0f
        );
    }
}

