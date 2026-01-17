#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "BlackboardBase.h"
#include "Nodes.h"
#include "Root.h"

class BehaviorTree
{
public:
    BehaviorTree(const std::string& name) : m_Owner(nullptr), m_Blackboard(nullptr), m_EditorApp(nullptr), m_Name(name) {}
    ~BehaviorTree();

    void StartTree();
    void TickTree();
    void StopTree();
    
    void SetRootNode(std::unique_ptr<HNode> root) { m_RootNode = std::move(root); }
    void SetNodeEditorApp(NodeEditorApp* editorApp) { m_EditorApp = editorApp; }
    void SetName(const std::string& name) { m_Name = name; }
    HNode* GetRootNode() const { return m_RootNode.get(); }
    HBlackboard* GetBlackboardRaw() const { return m_Blackboard.get(); }
    NodeEditorApp* GetEditorApp() const { return m_EditorApp; }
    const std::string& GetName() const { return m_Name; }

    template<typename OwnerType>
    void SetOwner(OwnerType* owner)
    {
        m_Owner = static_cast<void*>(owner);
    }
    template<typename OwnerType>
    OwnerType* GetOwner() const
    {
        return static_cast<OwnerType*>(m_Owner);
    }
private:
    bool m_bOwnsBlackboard = false;
    bool m_bIsRunning = false;
    
    void* m_Owner;
    std::string m_Name;
    
    std::unique_ptr<HNode> m_RootNode;
    std::unique_ptr<HBlackboard> m_Blackboard;
    NodeEditorApp* m_EditorApp;

    friend class BehaviorTreeBuilder;
};
template<typename OwnerType>
OwnerType* HNode::GetOwner() const
{
    return m_Tree ? m_Tree->GetOwner<OwnerType>() : nullptr;
}

class BehaviorTreeBuilder
{
public:
    BehaviorTreeBuilder() : m_Tree(Root::CreateEditorBehaviorTree("BehaviorTree")) {}
    BehaviorTreeBuilder(BehaviorTree* tree) : m_Tree(tree) {}

    template<typename BlackboardType>
    BehaviorTreeBuilder& setBlackboard()
    {
        static_assert(std::is_base_of_v<HBlackboard, BlackboardType>, "BlackboardType must derive from HBlackboard");
        //auto blackboard = new BlackboardType();
        auto blackboard = std::make_unique<BlackboardType>();
        m_Tree->m_Blackboard = std::move(blackboard);
        m_Tree->m_bOwnsBlackboard = true;
        return *this;
    }
    BehaviorTreeBuilder& setBlackboard(std::unique_ptr<HBlackboard> blackboard)
    {
        m_Tree->m_Blackboard = std::move(blackboard);
        m_Tree->m_bOwnsBlackboard = true;
        return *this;
    }
    BehaviorTreeBuilder& root();
    BehaviorTreeBuilder& sequence(const std::string& name);
    BehaviorTreeBuilder& selector(const std::string& name);
    template<typename ActionNodeType, typename... Args>
    BehaviorTreeBuilder& action(Args&&... args)
    {
        static_assert(std::is_base_of_v<HActionNode, ActionNodeType>, "ActionNodeType must derive from HAction");
        auto action = std::make_unique<ActionNodeType>(std::forward<Args>(args)...);
        
        m_LastCreatedNode = action.get();
        if (m_CurrentDecorator)
        {
            auto decoratorNode = std::move(m_CurrentDecorator);
            auto decoratorNodePtr = decoratorNode.get();
            if (!m_NodeStack.empty())
            {
                action->SetTree(m_Tree);
                action->SetType(HNodeType::Action);
                decoratorNode->AddChild(std::move(action));
                m_NodeStack.back()->AddChild(std::move(decoratorNode));
            }
        }
        else
            if (!m_NodeStack.empty())
            {
                action->SetTree(m_Tree);
                action->SetType(HNodeType::Action);
                m_NodeStack.back()->AddChild(std::move(action));
            }
        return *this;
    }
    template<typename ConditionNodeType, typename... Args>
    BehaviorTreeBuilder& condition(PriorityType priority, Args&&... args)
    {
        static_assert(std::is_base_of_v<HCondition, ConditionNodeType>, "ConditionNodeType must derive from HCondition");
        auto condition = std::make_unique<ConditionNodeType>(std::forward<Args>(args)...);
        
        if (m_LastCreatedNode)
        {
            condition->SetTree(m_Tree);
            condition->SetPriorityMode(priority);
            condition->SetType(HNodeType::Condition);
            m_LastCreatedNode->AddConditionNode(std::move(condition));
        }
        return *this;
    }
    template<typename DecoratorNodeType, typename... Args>
    BehaviorTreeBuilder& decorator(Args&&... args)
    {
        static_assert(std::is_base_of_v<HDecorator, DecoratorNodeType>, "DecoratorNodeType must derive from HDecorator");
        m_CurrentDecorator = std::make_unique<DecoratorNodeType>(std::forward<Args>(args)...);
        m_CurrentDecorator->SetTree(m_Tree);
        m_CurrentDecorator->SetType(HNodeType::Decorator);
        return *this;
    }
    BehaviorTreeBuilder& end();
    BehaviorTree* build() const;

    const HNode* GetLastCreatedNode() const { return m_LastCreatedNode; }
private:
    BehaviorTree* m_Tree;
    HNode* m_LastCreatedNode = nullptr;
    std::unique_ptr<HDecorator> m_CurrentDecorator;
    std::vector<HNode*> m_NodeStack;
};