#include "Tree.h"
#include "CompositeNodes.h"

//BehaviorTree methods
BehaviorTree* BehaviorTreeBuilder::build() const 
{
    return m_Tree;
}

BehaviorTree::~BehaviorTree()
{
    m_Blackboard = nullptr;
}

void BehaviorTree::StartTree()
{
    m_bIsRunning = true;
}

void BehaviorTree::TickTree()
{
    if (m_RootNode && m_bIsRunning && m_Blackboard)
    {
        m_RootNode->Tick();
        m_Blackboard->ClearValuesChangedFlag();
    }
}

void BehaviorTree::StopTree()
{
    if (!m_bIsRunning)
        return;
    m_bIsRunning = false;
    if (m_RootNode)
        m_RootNode->OnAbort();
}

// BehaviorTreeBuilder methods

BehaviorTreeBuilder& BehaviorTreeBuilder::root()
{
    auto rootNode = MakeNode<HRootNode>();
    rootNode->SetTree(m_Tree);
    
    HRootNode* rootNodePtr = rootNode.get();
    rootNodePtr->SetType(HNodeType::Root);
    
    m_LastCreatedNode = rootNodePtr;
    
    m_Tree->SetRootNode(std::move(rootNode));
    m_NodeStack.push_back(rootNodePtr);
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::sequence(const std::string& name)
{
    auto sequenceNode = MakeNode<SequenceNode>(name);
    
    SequenceNode* sequenceNodePtr = sequenceNode.get();
    sequenceNodePtr->SetType(HNodeType::Composite);
    sequenceNodePtr->SetTree(m_Tree);
    
    m_LastCreatedNode = sequenceNodePtr;
    if (m_CurrentDecorator)
    {
        auto decoratorNode = std::move(m_CurrentDecorator);
        decoratorNode->AddChild(std::move(sequenceNode));
        m_NodeStack.back()->AddChild(std::move(decoratorNode));
        m_NodeStack.push_back(sequenceNodePtr);
    }
    else
    {
        m_NodeStack.back()->AddChild(std::move(sequenceNode));
        m_NodeStack.push_back(sequenceNodePtr);
    }
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::selector(const std::string& name)
{
    auto selectorNode = MakeNode<SelectorNode>(name);
    auto selectorNodePtr = selectorNode.get();
    
    selectorNodePtr->SetType(HNodeType::Composite);
    selectorNodePtr->SetTree(m_Tree);
    
    m_LastCreatedNode = selectorNodePtr;
    if (m_CurrentDecorator)
    {
        auto decoratorNode = std::move(m_CurrentDecorator);
        decoratorNode->AddChild(std::move(selectorNode));
        m_NodeStack.back()->AddChild(std::move(decoratorNode));
        m_NodeStack.push_back(selectorNodePtr);
    }
    else
    {
        m_NodeStack.back()->AddChild(std::move(selectorNode));
        m_NodeStack.push_back(selectorNodePtr);
    }
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::end()
{
    if (!m_NodeStack.empty())
        m_NodeStack.pop_back();
    return *this;
}
