#pragma once
#include "Nodes.h"

class HCompositeNode : public HNode
{
public:
    HCompositeNode(const std::string& name) : HNode(name) {}
    
    bool CheckConditions();
    
    bool CanStart() override;
protected:
    int m_CurrentChildIndex = 0;
};

class SequenceNode : public HCompositeNode
{
public:
    SequenceNode(const std::string& name) : HCompositeNode(name) {}
    
    void OnStart() override;
    NodeStatus Update() override;
    void OnFinished() override;
    void OnAbort() override;
};

class SelectorNode : public HCompositeNode
{
public:
    SelectorNode(const std::string& name) : HCompositeNode(name) {}

    void OnStart() override;
    NodeStatus Update() override;
    void OnFinished() override;
    void OnAbort() override;
};
