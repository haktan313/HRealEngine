#pragma once
#include <unordered_map>
#include "NodeEditorHelper.h"

class NodeEditorApp
{
public:
    /*void AddActiveNode(HNode* node) { m_ActiveNodes.push_back(node); }
    void RemoveActiveNode(HNode* node) { m_ActiveNodes.erase(std::remove(m_ActiveNodes.begin(), m_ActiveNodes.end(), node), m_ActiveNodes.end());}
    void ClearActiveNodes() { m_ActiveNodes.clear(); }*/
    void ClearNodeMappings() { m_NodeToEditorIdMap.clear(); }
    void ClearDatas();
    
    using RuntimeNodeSyncer = std::function<void(HNode*, void*)>;
    static void SetRuntimeNodeSyncer(RuntimeNodeSyncer fn)
    {
        s_RuntimeNodeSyncer = fn;
    }
    virtual void DeserializeManagedActionParams(int nodeKey, const std::string& className, const YAML::Node& paramsNode) {}
    virtual void DeserializeManagedDecoratorParams(int nodeKey, const std::string& className, const YAML::Node& paramsNode) {}
    virtual void DeserializeManagedConditionParams(int nodeKey, const std::string& className, const YAML::Node& paramsNode) {}
private:
    static RuntimeNodeSyncer s_RuntimeNodeSyncer;
public:
    NodeEditorApp();
    virtual ~NodeEditorApp();
    void SetEmbeddedMode(bool enabled) { m_bIsEmbedded = enabled; }
    void SetRuntimeMode(bool enabled) { m_bIsRuntimeMode = enabled; }
    void DrawToolbar();
    void DrawGraph();
    void DrawBlackboard();
    void DrawDebugBehaviorTree();
    
    void OnStart();
    void Update();

    void RegisterNodeMapping(const HNode* runtimeNode, nodeEditor::NodeId editorId);
    Node* GetEditorNodeFor(const HNode* runtimeNode);
    const HNode* GetRuntimeNodeFor(nodeEditor::NodeId editorId);
    std::unordered_map<const HNode*, nodeEditor::NodeId>& GetNodeMappings() { return m_NodeToEditorIdMap; }
    NodeEditorHelper& GetNodeEditorHelper() { return *m_NodeEditor; }
    HBlackboard& GetBlackboard() { return *m_Blackboard; }
    bool IsRuntimeMode() const { return m_bIsRuntimeMode; }

    virtual void* GetManagedActionParams(int nodeKey) const { return nullptr; }
    virtual void* GetManagedDecoratorParams(int nodeKey) const { return nullptr; }
    virtual void* GetManagedConditionParams(int nodeKey) const { return nullptr; }
    
    void DecoratorNodeSelected(EditorDecorator& decorator);
    void ConditionNodeSelected(EditorCondition& condition);
    void DecoratorNodeUnSelected();
    void ConditionNodeUnSelected();
protected:
    std::unique_ptr<HBlackboard> m_Blackboard;

    std::unordered_map<int, std::string> m_NodeToActionClassId;
    std::unordered_map<int, std::unique_ptr<ParamsForAction>> m_NodeToParams;
    
    std::unordered_map<int, std::string> m_NodeToDecoratorClassId;
    std::unordered_map<int, std::unique_ptr<ParamsForDecorator>> m_NodeToDecoratorParams;

    std::unordered_map<int, std::string> m_NodeToConditionClassId;
    std::unordered_map<int, std::unique_ptr<ParamsForCondition>> m_NodeToConditionParams;
private:
    
    void MouseInputHandling();
    void NodeSettingsPanel();
    void FlowLinks();

    void BlackboardPanelSizeSettings();
    void BlackboardPanel();
    void ShowActionNodeInBlackboard();
    void ShowDecoratorNodeInBlackboard();
    void ShowConditionNodeInBlackboard();
    void ShowBlackboardDetails();
    HBlackboard& SetBlackboardForEditor(const std::string& id, const BlackboardClassInfo& info);
    std::unique_ptr<HBlackboard> GetUniqueBlackboard() { return std::move(m_Blackboard); }

    //For c# scripted nodes, we need to draw parameters in a different way, so we have separate functions for them
    virtual void DrawActionNodeParameters(int nodeKey, const std::string& classId);
    virtual void DrawDecoratorNodeParameters(int nodeKey, const std::string& classId);
    virtual void DrawConditionNodeParameters(int nodeKey, const std::string& classId);
    
    BehaviorTree* BuildBehaviorTree();
    void BuildSequence(Node* node, BehaviorTreeBuilder& btBuilder);
    void BuildSelector(Node* node, BehaviorTreeBuilder& btBuilder);
    void BuildAction(Node* node, BehaviorTreeBuilder& btBuilder);
    void ClearBuildData();
    void BuildPlanForNode(Node* editorNode, std::vector<BuildOp>& ops);
    std::vector<BuildOp> CreateBuildPlan();
    void CreateEditorTreeFromRuntimeTree(BehaviorTree* runtimeTree);

    bool m_bIsRuntimeMode = false;
    bool m_bDecoratorSelected = false;
    bool m_bConditionSelected = false;
    bool m_bIsEmbedded = false;
    void DrawBlackboardContent();
    
    float s_RightPanelWidth = 320.0f;
    
    /*std::vector<HNode*> m_ActiveNodes;*/

    std::string m_SelectedDebugTreeName = "Select Tree";
    EditorDecorator* m_LastSelectedDecorator = nullptr;
    EditorCondition* m_LastSelectedCondition = nullptr;
    Node* m_LastHoveredNode = nullptr;
    Node* m_LastSelectedNode = nullptr;
    BehaviorTree* m_BehaviorTree = nullptr;
    HBlackboard* m_CopyBlackboard = nullptr;
    std::unique_ptr<NodeEditorHelper> m_NodeEditor;
    
    std::unordered_map<const HNode*, nodeEditor::NodeId> m_NodeToEditorIdMap;
    std::unordered_map<uintptr_t, const HNode*> m_EditorIdToNodeMap;

    //std::unordered_map<int, std::string> m_NodeToActionClassId;
    //std::unordered_map<int, std::unique_ptr<ParamsForAction>> m_NodeToParams;
    std::string m_SelectedActionClassName;

    //std::unordered_map<int, std::string> m_NodeToDecoratorClassId;
    //std::unordered_map<int, std::unique_ptr<ParamsForDecorator>> m_NodeToDecoratorParams;
    std::string m_SelectedDecoratorClassName;

    //std::unordered_map<int, std::string> m_NodeToConditionClassId;
    //std::unordered_map<int, std::unique_ptr<ParamsForCondition>> m_NodeToConditionParams;
    std::string m_SelectedConditionClassName;

    std::string m_SelectedBlackboardClassName;

    std::string m_CurrentBTFilePath;

    friend class BTSerializer;
};
