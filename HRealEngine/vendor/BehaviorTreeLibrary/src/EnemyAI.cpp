#include "EnemyAI.h"
#include "CustomThings/CustomActions.h"
#include "CustomThings/CustomBlackboards.h"
#include "CustomThings/CustomConditions.h"
#include "CustomThings/CustomDecorators.h"

EnemyAI::EnemyAI()
{
    BeginPlay();
}

EnemyAI::~EnemyAI()
{
    EndPlay(); 
} 

void EnemyAI::BeginPlay()
{
    std::cout << "Enemy AI Started" << std::endl;
    /*MoveToParameters moveToParams;
    moveToParams.bToPlayer = true;
    moveToParams.MoveSpeed = 5.0f;
    moveToParams.StopDistance = 15.0f;
    
    MeleeEnemyAttackActionParameters attackParams;
    attackParams.AttackDuration = 1.f;
    attackParams.AttackPowerKey = "AttackPower";
    
    ChangeResultOfTheNodeParameters changeResultOfTheNodeParams;
    changeResultOfTheNodeParams.NewResult = NodeStatus::FAILURE;
    
    IsPlayerInRangeParameters isPlayerInRangeParams;
    isPlayerInRangeParams.Range = 31.f;
    isPlayerInRangeParams.DistanceToPlayerKey = "DistanceToPlayer";
    isPlayerInRangeParams.Priority = PriorityType::Both;
    
    CooldownDecoratorParameters cooldownDecoratorParams;
    cooldownDecoratorParams.CooldownTime = 5.f;
    
    m_BehaviorTree = BehaviorTreeBuilder().setBlackboard<MeleeEnemyBlackboard>()
        .root(Root::GetNodeEditorApp())
            .sequence("Main Sequence")
                .decorator<ChangeResultOfTheNodeDecorator>("Change Result Of The Node", changeResultOfTheNodeParams)
                    .action<MoveToAction>("Move To Player Action", moveToParams)
                .decorator<CooldownDecorator>("Cooldown Decorator", cooldownDecoratorParams).action<MeleeEnemyAttackAction>("Melee Attack Action", attackParams)
                    .condition<IsPlayerInRangeCondition>(PriorityType::Both, "Is Player In Range", isPlayerInRangeParams)
            .end()
        .build();
    m_BehaviorTree->SetOwner(this);
    m_BehaviorTree->StartTree();*/
    
    /*BehaviorTree* tree = nullptr;
    auto btPath = PlatformUtilsBT::OpenFile("Behavior Tree Files\0*.btree\0");
    BTSerializer serializer(tree);
    if (serializer.Deserialize(btPath))
    {
        std::cout << "Behavior Tree Loaded Successfully" << std::endl;
        m_BehaviorTree = tree;
        //auto newPath = PlatformUtilsBT::SaveFile("Behavior Tree Files\0*.btree\0");
        //serializer.Serialize(newPath);
        m_BehaviorTree->StartTree();
    }*/
        
}

void EnemyAI::Tick(float DeltaTime)
{
}

void EnemyAI::EndPlay()
{
    std::cout << "Enemy AI Ended" << std::endl;
}
