#pragma once
#include <memory>
#include <string>
#include <vector>
#include "EnumsStructsForTree.h"
#include "BlackboardBase.h"

class BehaviorTree;
class HCondition;
class NodeEditorApp;

class HNode
{
public:
    HNode(const std::string& name)
        : m_Name(name), m_Parent(nullptr), m_Status(NodeStatus::FAILURE), /*m_EditorApp(nullptr),*/ m_Type(HNodeType::None)
    {
        m_Params = std::make_unique<Params>();
    }
    virtual ~HNode() = default;
    
    NodeStatus Tick();
    virtual void OnStart();
    virtual NodeStatus Update() = 0;
    virtual void OnFinished();
    virtual void OnAbort();
    
    virtual void AddChild(std::unique_ptr<HNode> child);
    virtual void AddConditionNode(std::unique_ptr<HCondition> conditionNode);

    virtual bool CanStart() { return true; }
    
    void SetParent(HNode* parent) { m_Parent = parent; }
    void SetTree(BehaviorTree* tree) { m_Tree = tree; }
    void SetType(HNodeType type) { m_Type = type; }

    HNode* GetParent() const { return m_Parent; }
    BehaviorTree* GetTree() const { return m_Tree; }
    HBlackboard& GetBlackboard() const;
    NodeStatus GetStatus() const { return m_Status; }
    HNodeType GetType() const { return m_Type; }
    const Params& GetParams() const { return *m_Params; }
    uint64_t GetID() const { return m_ID; }
    
    const std::string& GetName() const { return m_Name; }
    const std::vector<std::unique_ptr<HNode>>& GetChildrensUnique() const { return m_Childrens; }
    const std::vector<HNode*> GetChildrensRaw() const
    {
        std::vector<HNode*> rawChildrens;
        for (const auto& child : m_Childrens)
            rawChildrens.push_back(child.get());
        return rawChildrens;
    }
    const std::vector<std::unique_ptr<HCondition>>& GetConditionNodesUnique() const { return m_ConditionNodes; }
    const std::vector<HCondition*> GetConditionNodesRaw() const
    {
        std::vector<HCondition*> rawConditions;
        for (const auto& condition : m_ConditionNodes)
            rawConditions.push_back(condition.get());
        return rawConditions;
    }
    template<typename T>
    void SetParams(const T& params) 
    { 
        m_Params = std::make_unique<T>(params); 
    }
    template<typename OwnerType>
    OwnerType* GetOwner() const;

    bool CheckConditionsSelfMode(HNode* node, const std::vector<std::unique_ptr<HCondition>>& conditionNodes);
    void CheckConditionsLowerPriorityMode(int& currentChildIndex, HNode* node, const std::vector<std::unique_ptr<HNode>>& childrens);

    bool m_bIsStarted = false;
protected:
    BehaviorTree* m_Tree = nullptr;
    HNode* m_Parent;
    std::unique_ptr<Params> m_Params;
    
    const std::string m_Name;
    NodeStatus m_Status;
    HNodeType m_Type;

    std::vector<std::unique_ptr<HNode>> m_Childrens;
    std::vector<std::unique_ptr<HCondition>> m_ConditionNodes;
    
    friend class BehaviorTreeBuilder;
private:
    uint64_t m_ID = 0;
    void SetID(uint64_t id) { m_ID = id; }
};

class HRootNode : public HNode
{
public:
    HRootNode() : HNode("Root") {}

    void OnStart() override;
    NodeStatus Update() override;
    void OnFinished() override;
    void OnAbort() override;

    void AddChild(std::unique_ptr<HNode> child) override;
    void AddConditionNode(std::unique_ptr<HCondition> conditionNode) override {}
};


struct ParamsForAction : Params
{
    ParamsForAction() = default;
    ~ParamsForAction() = default;

    virtual void DrawImGui(HBlackboard* blackboard) override {}
    virtual void Serialize(YAML::Emitter& out) const override {}
    virtual void Deserialize(const YAML::Node& node) {}
};
class HActionNode : public HNode
{
public:
    HActionNode(const std::string& name, const ParamsForAction& params = ParamsForAction{})
    : HNode(name) {}

    virtual void OnStart() override;
    virtual NodeStatus Update() override;
    virtual void OnFinished() override;
    virtual void OnAbort() override;

    bool CanStart() override;
    
    bool CheckConditions();
    bool CheckConditionsSelfMode();
private:
    friend class BehaviorTreeBuilder;
    friend class BehaviorTree;
};

struct ParamsForCondition : Params
{
    ParamsForCondition() = default;
    ~ParamsForCondition() = default;

    virtual void DrawImGui(HBlackboard* blackboard) override {}

    PriorityType Priority = PriorityType::None;
};
class HCondition : public HNode
{
public:
    HCondition(const std::string& name, const ParamsForCondition& params = ParamsForCondition{})
        : HNode(name), m_PriorityMode(PriorityType::None), m_LastStatus(NodeStatus::RUNNING) {}

    virtual void OnStart() override {}
    virtual bool CheckCondition() = 0;
    virtual void OnFinished() override { m_bIsStarted = false; }
    virtual void OnAbort() override { HNode::OnAbort(); }

    void SetLastStatus(NodeStatus status) { m_LastStatus = status; }

    PriorityType GetPriorityMode() const { return m_PriorityMode; }
    NodeStatus GetLastStatus() const { return m_LastStatus; }
private:
    PriorityType m_PriorityMode;
    NodeStatus m_LastStatus;

    NodeStatus Update() override final { return CheckCondition() ? NodeStatus::SUCCESS : NodeStatus::FAILURE; }
    void SetPriorityMode(PriorityType priority) { m_PriorityMode = priority; }
    friend class BehaviorTreeBuilder;
    friend class BehaviorTree;
};

struct ParamsForDecorator : public Params
{
    ParamsForDecorator() = default;
    ~ParamsForDecorator() = default;

    virtual void DrawImGui(HBlackboard* blackboard) override {}
}; 
class HDecorator : public HNode
{
public:
    HDecorator(const std::string& name, const ParamsForDecorator& params = ParamsForDecorator{})
    : HNode(name) {}

    virtual void OnStart() override {}
    virtual bool CanExecute() = 0;
    virtual void OnFinishedResult(NodeStatus& status) = 0;
    virtual void OnFinished() override { m_bIsStarted = false; }
    virtual void OnAbort() override { HNode::OnAbort(); }
private:
    virtual NodeStatus Update() override final;
    friend class BehaviorTreeBuilder;
    friend class BehaviorTree;
};