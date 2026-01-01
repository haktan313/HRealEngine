#include "HRpch.h"
#include "Asset.h"

namespace HRealEngine
{
    std::string_view AssetTypeToString(AssetType type)
    {
        switch (type)
        {
            case AssetType::None:
                return "AssetType::None";
            case AssetType::Scene:
                return "AssetType::Scene";
            case AssetType::Texture:
                return "AssetType::Texture";
            case AssetType::Mesh:
                return "AssetType::Mesh";
            case AssetType::Material:
                return "AssetType::Material";
        }

        return "AssetType::<Invalid>";
    }

    AssetType AssetTypeFromString(std::string_view assetType)
    {
        if (assetType == "AssetType::None")
            return AssetType::None;
        if (assetType == "AssetType::Scene")
            return AssetType::Scene;
        if (assetType == "AssetType::Texture")
            return AssetType::Texture;
        if (assetType == "AssetType::Mesh")
            return AssetType::Mesh;
        if (assetType == "AssetType::Material")
            return AssetType::Material;

        return AssetType::None;
    }
}
