#include "CompositeNodes.h"
#include <iostream>
#include "Tree.h"
#include "Editor/NodeEditorApp.h"

// HCompositeNode methods
bool HCompositeNode::CheckConditions()
{
    if (!m_ConditionNodes.empty())
        for (auto& condition : m_ConditionNodes)
        {
            NodeStatus conditionStatus = condition->Tick();
            if (conditionStatus == NodeStatus::FAILURE)
            {
                m_Status = NodeStatus::FAILURE;
                return false;
            }
        }
    
    return true;
}

bool HCompositeNode::CanStart()
{
    if (!CheckConditions())
    {
        m_bIsStarted = false;
        m_Status = NodeStatus::FAILURE;
        return false;
    }
    return true;
}

// SequenceNode methods
void SequenceNode::OnStart()
{
    /*if (GetTree() && GetTree()->GetEditorApp())
        GetTree()->GetEditorApp()->AddActiveNode(this);*/
    if (GetTree())
        GetTree()->AddActiveNode(this);
}

NodeStatus SequenceNode::Update()
{
    if (!CheckConditionsSelfMode(this, m_ConditionNodes))
        return NodeStatus::FAILURE;
    
    while (m_CurrentChildIndex < static_cast<int>(m_Childrens.size()))
    {
        NodeStatus status = m_Childrens[m_CurrentChildIndex]->Tick();
        if (status == NodeStatus::RUNNING)
            return NodeStatus::RUNNING;
        if (status == NodeStatus::FAILURE)
        {
            m_CurrentChildIndex = 0;
            return NodeStatus::FAILURE;
        }
        m_CurrentChildIndex++;
    }
    m_CurrentChildIndex = 0;
    return NodeStatus::SUCCESS;
}

void SequenceNode::OnFinished()
{
    m_CurrentChildIndex = 0;
    m_bIsStarted = false;
    /*if (GetTree() && GetTree()->GetEditorApp())
        GetTree()->GetEditorApp()->RemoveActiveNode(this);*/
    if (GetTree())
        GetTree()->RemoveActiveNode(this);
}

void SequenceNode::OnAbort()
{
    if (m_CurrentChildIndex < static_cast<int>(m_Childrens.size()))
        if (m_Childrens[m_CurrentChildIndex]->GetStatus() == NodeStatus::RUNNING)
            m_Childrens[m_CurrentChildIndex]->OnAbort();
    m_CurrentChildIndex = 0;
    m_bIsStarted = false;
    m_Status = NodeStatus::FAILURE;
}

//SelectorNode methods
void SelectorNode::OnStart()
{
    /*if (GetTree() && GetTree()->GetEditorApp())
        GetTree()->GetEditorApp()->AddActiveNode(this);*/
    if (GetTree())
        GetTree()->AddActiveNode(this);
}

NodeStatus SelectorNode::Update()
{
    if (!CheckConditionsSelfMode(this, m_ConditionNodes))
        return NodeStatus::FAILURE;
    CheckConditionsLowerPriorityMode(m_CurrentChildIndex, this, m_Childrens);
    while (m_CurrentChildIndex < static_cast<int>(m_Childrens.size()))
    {
        NodeStatus status = m_Childrens[m_CurrentChildIndex]->Tick();
        if (status == NodeStatus::RUNNING)
            return NodeStatus::RUNNING;
        if (status == NodeStatus::FAILURE)
        {
            m_CurrentChildIndex++;
            continue;
        }
        if (status == NodeStatus::SUCCESS)
        {
            m_CurrentChildIndex = 0;
            return NodeStatus::SUCCESS;
        }
    }
    m_CurrentChildIndex = 0;
    return NodeStatus::FAILURE;
}

void SelectorNode::OnFinished()
{
    m_CurrentChildIndex = 0;
    m_bIsStarted = false;
    /*if (GetTree() && GetTree()->GetEditorApp())
        GetTree()->GetEditorApp()->RemoveActiveNode(this);*/
    if (GetTree())
        GetTree()->RemoveActiveNode(this);
}

void SelectorNode::OnAbort()
{
    if (m_CurrentChildIndex < static_cast<int>(m_Childrens.size()))
        if (m_Childrens[m_CurrentChildIndex]->GetStatus() == NodeStatus::RUNNING)
            m_Childrens[m_CurrentChildIndex]->OnAbort();
    m_CurrentChildIndex = 0;
    m_bIsStarted = false;
    m_Status = NodeStatus::FAILURE;
}