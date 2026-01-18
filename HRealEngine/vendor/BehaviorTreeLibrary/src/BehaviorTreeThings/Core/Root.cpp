#include "Root.h"
#include "Tree.h"

std::unordered_map<BehaviorTree*, std::string> Root::m_BehaviorTreeMap;
std::vector<BehaviorTree*> Root::m_BehaviorTrees;
BehaviorTree* Root::m_EditorBehaviorTree = nullptr;

void Root::RootStart()
{
    for (BehaviorTree* tree : m_BehaviorTrees)
        tree->StartTree();
}

void Root::RootTick()
{
    auto amount = m_BehaviorTrees.size();
    std::cout << "Ticking " << amount << " behavior trees." << std::endl;
    for (BehaviorTree* tree : m_BehaviorTrees)
        tree->TickTree();
}

void Root::RootClear()
{
    for (BehaviorTree* tree : m_BehaviorTrees)
    {
        tree->StopTree();
        delete tree;
    }
    m_BehaviorTrees.clear();
}

void Root::RootStop()
{
    for (BehaviorTree* tree : m_BehaviorTrees)
        tree->StopTree();
}

BehaviorTree* Root::CreateBehaviorTree(const std::string& name)
{
    std::string finalName = name;
    int counter = 1;
    bool exists = true;

    while (exists)
    {
        exists = false;

        for (const auto* tree : m_BehaviorTrees)
        {
            if (tree->GetName() == finalName)
            {
                exists = true;
                finalName = name + "_" + std::to_string(counter);
                ++counter;
                break;
            }
        }
    }
    
    BehaviorTree* tree = new BehaviorTree(finalName);
    m_BehaviorTrees.push_back(tree);
    return tree;
}

BehaviorTree* Root::CreateBehaviorTree(const std::string& name, const std::string& path)
{
    auto tree = CreateBehaviorTree(name);
    m_BehaviorTreeMap[tree] = path;
    return tree;
}

BehaviorTree* Root::CreateEditorBehaviorTree(const std::string& name)
{
    if (m_EditorBehaviorTree)
        DestroyEditorBehaviorTree();
    
    m_EditorBehaviorTree = new BehaviorTree(name);
    return m_EditorBehaviorTree;
}

void Root::DestroyEditorBehaviorTree()
{
    if (m_EditorBehaviorTree)
    {
        m_EditorBehaviorTree->StopTree();
        delete m_EditorBehaviorTree;
        m_EditorBehaviorTree = nullptr;
    }
}

std::string Root::GetBehaviorTreePath(BehaviorTree* tree)
{
    if (m_BehaviorTreeMap.find(tree) != m_BehaviorTreeMap.end())
        return m_BehaviorTreeMap[tree];
    return "";
}

void Root::DestroyBehaviorTree(BehaviorTree* tree)
{
    if (tree)
    {
        tree->StopTree();
        auto it = std::find(m_BehaviorTrees.begin(), m_BehaviorTrees.end(), tree);
        if (it != m_BehaviorTrees.end())
            m_BehaviorTrees.erase(it);
        delete tree;
    }
}
