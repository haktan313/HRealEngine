#pragma once
#include "HRealEngine/Asset/Asset.h"
#include "HRealEngine/Asset/AssetSystemBase.h"

namespace HRealEngine
{
    class BehaviorTreeAsset : public Asset
    {
    public:
        BehaviorTreeAsset() = default;
        ~BehaviorTreeAsset() = default;

        static AssetType GetStaticType() { return AssetType::BehaviorTree; }
        AssetType GetType() const override { return GetStaticType(); }
    };
}
