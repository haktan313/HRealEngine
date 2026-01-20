# Behavior Tree Library
Behavior Tree library provides a framework in C++ with visual and script based tree creation, blackboards, actions, decorators, conditions, and live node flow debugging.
The node visuals are created using **imgui-node-editor**. You can find **example implementations** in the **CustomThings** folder.
- For cloning repo `git clone https://github.com/haktan313/NavigationmeshSystem.git`
- Then run the Generation.bat file

## Features
- Visual **Behavior Tree editor** built with **ImGui** + **imgui-node-editor**
- **YAML Serialization Support:** Save and load behavior trees (visual nodes + runtime logic) to `.btree` files using **yaml-cpp**
- **Parameter Persistence:** Preserves node parameters, blackboard values, and connections between sessions
- Script based and visual **Behavior Tree** creation in editor window
- Extensible system with **custom Actions**, **Decorators**, and **Conditions**
- **Blackboard** driven AI architecture
- Runtime node execution **flow visualization** (live active node & link flow)
- **Condition priority system** (Self, Lower Priority, Both)
- **Decorator execution control** (cooldown, result override, etc.)
- Fully C++ based framework with clear node lifecycle (OnStart / Tick / OnFinish / CheckCondition / CanExecute / OnFinishedResult / OnAbort)
- Ability to assign custom blackboards, actions, decorators, and conditions directly in the editor
- Designed for game AI and experimentation

## Screenshots
| Visual Behavior Tree Creation | Script Base Behavior Tree Creation |
|--------|-------------|
| <img width="1912" height="1020" alt="image" src="https://github.com/user-attachments/assets/5615dd27-79a6-4559-ab26-d14b8c3dbd26" /> | <img width="1345" height="799" alt="Screenshot 2026-01-11 212614" src="https://github.com/user-attachments/assets/e309f3b7-4502-473c-8be9-9d99baaf3c05" /> |


## Usage Examples

### Creating a Blackboard
- Create a class derived from the **HBlackboard** base class.
- In its constructor, create blackboard values using the **CreateXValue** functions by assigning a **KeyName** and a **value**.
- After creating your **blackboard**, to register it call `AddBlackBoardToEditor<YourBlackboardClass>(Its name)` function before Start the system for example at `App.cpp`:
  ```cpp
  AddBlackBoardToEditor<MeleeEnemyBlackboard>("Melee Enemy Blackboard");
  ```
  ### HBlackboard
  - **HBlackboard** has its own **DrawImGui** function, allowing it to be visualized directly in the editor.
  - Provides **Get** and **Set** functions for accessing **blackboard values**. Also includes helper **Get** functions to **retrieve all variables of the same type**.
  - Provides **Has** functions for checking it has that value or not.
  - There is a **DrawImGui** function, with it you will see the varaibles visual at the editor.
  - When a value changes, an internal flag is set to true for **condition checking**. This allows the tree to skip condition checks unless values change, improving performance.

    ```cpp
    class MeleeEnemyBlackboard : public HBlackboard
    {
    public:
        MeleeEnemyBlackboard(const std::string& name = "MeleeEnemyBlackboard") : HBlackboard(name)
        {
            CreateBoolValue("IsPlayerInRange", false);
            CreateBoolValue("IsPlayerAttacking", false);
            CreateBoolValue("CanAttack", true);
            CreateBoolValue("IsAttacking", false);
            
            CreateFloatValue("DistanceToPlayer", 200.0f);
            CreateFloatValue("Health", 100.0f);
            CreateFloatValue("Stamina", 50.0f);
            
            CreateIntValue("AttackPower", 10);
            
            CreateStringValue("CurrentState", "Idle");
        }
    };
    
### Creating Actions
- Create a struct derived from the **ParamsForAction** base struct. This struct is used to define action parameters and is passed to the action class constructor.
- Create a class derived from the **HActionNode** base class.
- After creating your **action**, to register it call `AddActionNodeToBuilder<YourActionClass, YourActionParameterStruct>(Its name)` function:
    ```cpp
    AddActionNodeToBuilder<MeleeEnemyAttackAction, MeleeEnemyAttackActionParameters>("Melee Enemy Attack Action");
    ```
    ### HActionNode & ParamsForAction
  - **ParamsForAction** provides a **DrawImGui** function to expose action parameters directly in the editor. Just assign parameters with `DrawXValue` function inside `DrawImGui` function.
  - If you want to assign Blackboard key value as a parameter create varaible name `HBlackboardKeyValue` in the `DrawImGui` function call `DrawBlackboardIntKeySelector` function for visualize it. (as shown in the example bottom)
  - To Serialize and Deserialize its parameters call `Serialize` and `Deserialize` functions. (as shown in the example bottom)
  - HActionNode provides helper functions:
    - `GetOwner<CustomAIClass>()` - access the owning AI / actor
    - `GetBlackboard()` – access the assigned blackboard
  ```cpp
    struct MeleeEnemyAttackActionParameters : ParamsForAction
    {
        HBlackboardKeyValue AttackPowerKey;
        float AttackDuration = 10.0f;
        void DrawImGui(HBlackboard* blackboard) override
        {
            DrawBlackboardIntKeySelector("Attack Power", AttackPowerKey, blackboard);
            DrawFloatValue("Attack Duration", AttackDuration);
        }
        void Serialize(YAML::Emitter& out) const override
        {
            SerializeBlackboardFloatKey("AttackPowerKey", AttackPowerKey, out);
            SerializeFloat("AttackDuration", AttackDuration, out);
        }
        void Deserialize(const YAML::Node& node) override
        {
            DeserializeBlackboardKey(node, "AttackPowerKey", AttackPowerKey);
            DeserializeFloat(node, "AttackDuration", AttackDuration);
        }
    };
    class MeleeEnemyAttackAction : public HActionNode
    {
    public:
        MeleeEnemyAttackAction(const std::string& name, const MeleeEnemyAttackActionParameters& params = MeleeEnemyAttackActionParameters{})
            : HActionNode(name, params), m_AttackPowerKey(params.AttackPowerKey), m_AttackDuration(params.AttackDuration)
        {
            SetParams<MeleeEnemyAttackActionParameters>(params);
        }

        void OnStart() override;
        NodeStatus Update() override;
        void OnFinished() override;
        void OnAbort() override;
    private:
        HBlackboardKeyValue m_AttackPowerKey;
        float m_AttackDuration;
        float m_ElapsedTime = 0.0f;
    };
  ```
  

### Creating Conditions
- Create a struct derived from the **ParamsForCondition** base struct. This struct is used to define action parameters and is passed to the action class constructor.
- Create a class derived from the **HCondition** base class.
- After creating your **condition**, to register it to the **editor**, go to `NodeEditorApp.cpp` and call `AddConditionNodeToBuilder<YourConditionClass, YourConditionParameterStruct>(Its name)` inside the **OnStart** function:
    ```cpp
    AddConditionNodeToBuilder<IsPlayerInRangeCondition, IsPlayerInRangeParameters>("Is Player In Range Condition");
    ```
    ### HCondition & ParamsForCondition
    - **ParamsForCondition** provides a **DrawImGui** function to expose action parameters directly in the editor. Just assign parameters with `DrawXValue` function inside `DrawImGui` function.
    - If you want to assign Blackboard key value as a parameter create varaible name `HBlackboardKeyValue` in the `DrawImGui` function call `DrawBlackboardIntKeySelector` function for visualize it. (as shown in the example bottom).
    - To Serialize and Deserialize its parameters call `Serialize` and `Deserialize` functions. (as shown in the example bottom)
    - HCondition provides helper functions:
        - `GetOwner<CustomAIClass>()` – access the owning AI / actor
        - `GetBlackboard()` – access the assigned blackboard
        - `GetPriorityMode()` - returns the condition’s priority mode
        - `GetLastStatus()` - returns the last evaluated status (Success / Failure)
    ```cpp
    struct IsPlayerInRangeParameters : ParamsForCondition
    {
        float Range = 100.0f;
        HBlackboardKeyValue DistanceToPlayerKey;
        void DrawImGui(HBlackboard* blackboard) override
        {
            DrawFloatValue("Range", Range);
            DrawBlackboardFloatKeySelector("Distance To Player", DistanceToPlayerKey, blackboard);
        }
        void Serialize(YAML::Emitter& out) const override
        {
            SerializeFloat("Range", Range, out);
            SerializeBlackboardFloatKey("DistanceToPlayerKey", DistanceToPlayerKey, out);
        }
        void Deserialize(const YAML::Node& node) override
        {
            DeserializeFloat(node, "Range", Range);
            DeserializeBlackboardKey(node, "DistanceToPlayerKey", DistanceToPlayerKey);
        }
    };
    class IsPlayerInRangeCondition : public HCondition
    {
    public:
        IsPlayerInRangeCondition(const std::string& name, const IsPlayerInRangeParameters& params = IsPlayerInRangeParameters{})
            : HCondition(name, params), m_Range(params.Range), m_DistanceToPlayer(params.DistanceToPlayerKey)
        {
            SetParams<IsPlayerInRangeParameters>(params);
        }

        void OnStart() override;
        bool CheckCondition() override;
        void OnFinished() override;
        void OnAbort() override;
    private:
        float m_Range;
        HBlackboardKeyValue m_DistanceToPlayer;
    };
    ```

### Creating Decorators
- Create a struct derived from the **ParamsForDecorator** base struct. This struct is used to define action parameters and is passed to the action class constructor.
- Create a class derived from the **HDecorator** base class.
- After creating your **decorator**, to register it call `AddDecoratorNodeToBuilder<YourDecoratorClass, YourDecoratorParameterStruct>(Its name)` function:
    ```cpp
    AddDecoratorNodeToBuilder<ChangeResultOfTheNodeDecorator, ChangeResultOfTheNodeParameters>("Change Result Of The Node Decorator");
    ```
    ```cpp
    struct ChangeResultOfTheNodeParameters : ParamsForDecorator
    {
        NodeStatus NewResult = NodeStatus::SUCCESS;
        void DrawImGui(HBlackboard* blackboard) override
        {
            const char* items[] = { "SUCCESS", "FAILURE", "RUNNING" };
            int currentItem = static_cast<int>(NewResult);
            if (ImGui::Combo("New Result", &currentItem, items, IM_ARRAYSIZE(items)))
            {
                NewResult = static_cast<NodeStatus>(currentItem);
            }
        }
        void Serialize(YAML::Emitter& out) const override
        {
            SerializeInt("NewResult", static_cast<int>(NewResult), out);
        }
        void Deserialize(const YAML::Node& node) override
        {
            int resultInt = 0;
            DeserializeInt(node, "NewResult", resultInt);
            NewResult = static_cast<NodeStatus>(resultInt);
        }
    };
    class ChangeResultOfTheNodeDecorator : public HDecorator
    {
    public:
        ChangeResultOfTheNodeDecorator(const std::string& name, const ChangeResultOfTheNodeParameters& params = ChangeResultOfTheNodeParameters{})
            : HDecorator(name, params), m_NewResult(params.NewResult)
        {
            SetParams<ChangeResultOfTheNodeParameters>(params);
        }
        void OnStart() override;
        bool CanExecute() override;
        void OnFinishedResult(NodeStatus& status) override;
        void OnFinished() override;
        void OnAbort() override;
    private:
        NodeStatus m_NewResult;
    };
    ```
    ### HDecorator & ParamsForDecorator
    - **ParamsForDecorator** provides a **DrawImGui** function to expose action parameters directly in the editor. Just assign parameters with `DrawXValue` function inside `DrawImGui` function.
    - If you want to assign Blackboard key value as a parameter create varaible name `HBlackboardKeyValue` in the `DrawImGui` function call `DrawBlackboardIntKeySelector` function for visualize it. (as shown in the example above).
    - To Serialize and Deserialize its parameters call `Serialize` and `Deserialize` functions. (as shown in the example above)
    - HDecorator provides helper functions:
        - `GetOwner<CustomAIClass>()` – access the owning AI / actor
        - `GetBlackboard()` – access the assigned blackboard

## Setup
- You can check the `App.cpp` file for setup. You can find the instructions here.
```cpp
    //-------------------------------------------- Changable Part ------------------------------------------------//
    // Root::RootStart() starts whole Behavior Trees.
    // Root::RootTick() call this inside the main loop.
    // Root::RootClear() call this before app shutdown.
    // Root::RootStop() stops whole Behavior Trees.
    EditorRoot::EditorRootStart();//Initialize the Node Editor App, if you want you can work without editor app for that don't call this function
    // EditorRoot::EditorRootStop() call this before app shutdown.
    // EditorRoot::GetNodeEditorApp() to get the instance of the Node Editor App.
    // EditorRoot::EditorRootTick() call this inside the main loop after Root::RootTick().

    //Register Custom Blackboard, Actions, Conditions and Decorators to the Node Registry
    NodeRegistry::AddBlackBoardToEditor<MeleeEnemyBlackboard>("Melee Enemy Blackboard");
    NodeRegistry::AddBlackBoardToEditor<RangedEnemyBlackboard>("Ranged Enemy Blackboard");
    
    NodeRegistry::AddActionNodeToBuilder<MoveToAction, MoveToParameters>("Move To Action");
    NodeRegistry::AddActionNodeToBuilder<MeleeEnemyAttackAction, MeleeEnemyAttackActionParameters>("Melee Enemy Attack Action");
    NodeRegistry::AddActionNodeToBuilder<HeavyAttackAction, HeavyAttackActionParameters>("Heavy Attack Action");
    
    NodeRegistry::AddConditionNodeToBuilder<IsPlayerInRangeCondition, IsPlayerInRangeParameters>("Is Player In Range Condition");
    NodeRegistry::AddConditionNodeToBuilder<CanAttackCondition, CanAttackParameters>("Can Attack Condition");
    
    NodeRegistry::AddDecoratorNodeToBuilder<ChangeResultOfTheNodeDecorator, ChangeResultOfTheNodeParameters>("Change Result Of The Node Decorator");
    NodeRegistry::AddDecoratorNodeToBuilder<CooldownDecorator, CooldownDecoratorParameters>("Cooldown Decorator");

    // PlatformUtilsBT::SetWindow(m_Window); set the window for using in the Behavior Trees platform utils.
    
    //you can find the example in the EnemyAI.cpp. "m_BehaviorTree->SetOwner(this);"
    m_EnemyAI = new EnemyAI();
    //-------------------------------------------- Changable Part ------------------------------------------------//
```
Script Example(Behavior Tree and Builder)

```cpp
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
    void AddActiveNode(HNode* node) { m_ActiveNodes.push_back(node); }
    void RemoveActiveNode(HNode* node) { m_ActiveNodes.erase(std::remove(m_ActiveNodes.begin(), m_ActiveNodes.end(), node), m_ActiveNodes.end());}
    void ClearActiveNodes() { m_ActiveNodes.clear(); }
    const std::vector<HNode*>& GetActiveNodes() const { return m_ActiveNodes; }
    
    bool m_bIsRunning = false;
    
    void* m_Owner;
    std::string m_Name;

    std::vector<HNode*> m_ActiveNodes;
    
    std::unique_ptr<HNode> m_RootNode;
    std::unique_ptr<HBlackboard> m_Blackboard;
    NodeEditorApp* m_EditorApp;

    friend class BehaviorTreeBuilder;
    friend class NodeEditorApp;
    friend class SequenceNode;
    friend class SelectorNode;
    friend class HRootNode;
    friend class HNode;
};
template<typename OwnerType>
OwnerType* HNode::GetOwner() const
{
    return m_Tree ? m_Tree->GetOwner<OwnerType>() : nullptr;
}

class BehaviorTreeBuilder
{
public:
    BehaviorTreeBuilder() : m_Tree(Root::CreateBehaviorTree("BehaviorTree")) {}
    BehaviorTreeBuilder(BehaviorTree* tree) : m_Tree(tree) {}

    template<typename BlackboardType>
    BehaviorTreeBuilder& setBlackboard()
    {
        static_assert(std::is_base_of_v<HBlackboard, BlackboardType>, "BlackboardType must derive from HBlackboard");
        //auto blackboard = new BlackboardType();
        auto blackboard = std::make_unique<BlackboardType>();
        m_Tree->m_Blackboard = std::move(blackboard);
        return *this;
    }
    BehaviorTreeBuilder& setBlackboard(std::unique_ptr<HBlackboard> blackboard)
    {
        m_Tree->m_Blackboard = std::move(blackboard);
        return *this;
    }
    BehaviorTreeBuilder& root();
    BehaviorTreeBuilder& sequence(const std::string& name);
    BehaviorTreeBuilder& selector(const std::string& name);
    template<typename ActionNodeType, typename... Args>
    BehaviorTreeBuilder& action(Args&&... args)
    {
        static_assert(std::is_base_of_v<HActionNode, ActionNodeType>, "ActionNodeType must derive from HAction");
        auto action = MakeNode<ActionNodeType>(std::forward<Args>(args)...);
        
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
        auto condition = MakeNode<ConditionNodeType>(std::forward<Args>(args)...);
        
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
        m_CurrentDecorator = MakeNode<DecoratorNodeType>(std::forward<Args>(args)...);
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
    
    uint64_t m_NextUID = 1;
    template<typename TNode, typename... Args>
    std::unique_ptr<TNode> MakeNode(Args&&... args)
    {
        auto node = std::make_unique<TNode>(std::forward<Args>(args)...);
        node->SetID(m_NextUID++);
        return node;
    }

};

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

```

## Future Goals
- Adding a simple Parallel Node
- Service node
- Subtree
