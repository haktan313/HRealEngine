#pragma once
#include <vector>
#include <xstring>
#include <unordered_map>

class BehaviorTree;

class Root
{
public:
    static void RootStart();
    static void RootTick();
    static void RootClear();
    static void RootStop();

    static BehaviorTree* CreateBehaviorTree(const std::string& name);
    static BehaviorTree* CreateBehaviorTree(const std::string& name, const std::string& path);
    static BehaviorTree* CreateEditorBehaviorTree(const std::string& name);
    static void DestroyBehaviorTree(BehaviorTree* tree);
    static void DestroyEditorBehaviorTree();
    static void AddBehaviorTree(BehaviorTree* tree) { m_BehaviorTrees.push_back(tree); }
    static void SetEditorBehaviorTree(BehaviorTree* tree) { m_EditorBehaviorTree = tree; }

    static std::vector<BehaviorTree*>& GetBehaviorTrees() { return m_BehaviorTrees; }
    static BehaviorTree* GetEditorBehaviorTree() { return m_EditorBehaviorTree; }
    static std::string GetBehaviorTreePath(BehaviorTree* tree);
private:
    static std::vector<BehaviorTree*> m_BehaviorTrees;
    static BehaviorTree* m_EditorBehaviorTree;
    static std::unordered_map<BehaviorTree*, std::string> m_BehaviorTreeMap;
};
