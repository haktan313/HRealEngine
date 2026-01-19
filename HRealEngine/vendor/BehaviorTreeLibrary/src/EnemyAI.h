#pragma once
#include <iostream>

class BehaviorTree;

class EnemyAI
{
public:
    EnemyAI();
    ~EnemyAI();

    void BeginPlay();
    void Tick(float DeltaTime);
    void EndPlay();
    void PrintStatus() { std::cout << "Test Value: " << m_TestValue << std::endl; }
    void SetTestValue(float value) { m_TestValue = value; }
    float GetTestValue() const { return m_TestValue; }
private:
    float m_TestValue = 0.0f;
    BehaviorTree* m_BehaviorTree;
};
