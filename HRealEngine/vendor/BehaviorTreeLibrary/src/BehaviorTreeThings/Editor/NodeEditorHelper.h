#pragma once
#include "NodeEditorStructsAndEnums.h"

class NodeEditorApp;
class NodeEditorHelper
{
    NodeEditorApp* m_App;
public:
    int GetNextID() { return m_NextId++; }
    //void TouchNode(nodeEditor::NodeId id) { m_NodeTouchTime[id] = m_TouchTime; }
    nodeEditor::LinkId GetNextLinkId() { return nodeEditor::LinkId(GetNextID()); }
    std::vector<Node>& GetNodes() { return m_Nodes; }
    std::vector<Link>& GetLinks() { return m_Links; }

    void ClearDatas();

    NodeEditorHelper(NodeEditorApp* app = nullptr);
    
    void OnStart();
    void OnUpdate();

    void SetActiveNode(Node* node) { m_ActiveNode = node; }

    Node* SpawnRootNode();
    Node* SpawnSequenceNode(ImVec2 position);
    Node* SpawnSelectorNode(ImVec2 position);
    Node* SpawnActionNode(ImVec2 position);
    void SpawnConditionNode(Node* parentNode);
    void SpawnDecoratorNode(Node* parentNode);
    
    std::vector<Node*> GetChilderenNodes(Node* parentNode);
    Node* GetSelectedNode();
    Node* FindNode(nodeEditor::NodeId id);
    Pin* FindPin(nodeEditor::PinId id);
    
    void BuildNodes();
private:
    void BuildNode(Node* node);
    
    void ManageInputs(ImRect& inputsRect, int& inputAlpha, Node& node, float padding);
    
    void DrawDecoratorBar(Node& node, ImRect& decoratorRect);
    void DrawConditionBar(Node& node, ImRect& conditionRect);
    
    void ManageOutputs(ImRect& outputsRect, int& outputAlpha, Node& node, float padding);
    void ManageLinks();
    bool CanCreateLink(Pin* a, Pin* b);
    
    void StylizeNodes();
    void PaintNodeBackground(Node& node, const ImRect& inputsRect, const ImRect& outputsRect, const ImRect& contentRect, const ImVec4& pinBackground, int inputAlpha, int outputAlpha, const ImRect& sequenceRect);
    
    int m_NextId = 1;
    //float m_TouchTime = 1.0f;
    Pin* newLinkPin = nullptr;
    Node* m_ActiveNode = nullptr;
    nodeEditor::EditorContext* m_EditorContext = nullptr;
    nodeEditor::PinId m_RootOutputPinId;
    std::vector<Node> m_Nodes;
    std::vector<Link> m_Links;
    //std::map<nodeEditor::NodeId, float, NodeIdLess> m_NodeTouchTime;
};
