#pragma once
#include <vector>

class BehaviorTree;

class Root
{
public:
    static void RootStart();
    static void RootTick();
    static void RootClear();
    static void RootStop();

    static BehaviorTree* CreateBehaviorTree();
    static void DestroyBehaviorTree(BehaviorTree* tree);
    static void AddBehaviorTree(BehaviorTree* tree) { m_BehaviorTrees.push_back(tree); }

    static std::vector<BehaviorTree*>& GetBehaviorTrees() { return m_BehaviorTrees; }
private:
    static std::vector<BehaviorTree*> m_BehaviorTrees;
};
