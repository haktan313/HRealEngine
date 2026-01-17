#include "Root.h"
#include "Tree.h"

std::vector<BehaviorTree*> Root::m_BehaviorTrees;

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