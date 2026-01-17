#include "NodeRegistry.h"

std::unordered_map<std::string, ActionClassInfo> NodeRegistry::s_ActionClassInfoMap;
std::unordered_map<std::string, DecoratorClassInfo> NodeRegistry::s_DecoratorClassInfoMap;
std::unordered_map<std::string, ConditionClassInfo> NodeRegistry::s_ConditionClassInfoMap;
std::unordered_map<std::string, BlackboardClassInfo> NodeRegistry::s_BlackboardClassInfoMap;