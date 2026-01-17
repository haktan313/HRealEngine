#pragma once
#include <functional>
#include <imgui_node_editor_internal.h>
#include "BehaviorTreeThings/Core/Nodes.h"

namespace nodeEditor = ax::NodeEditor;

enum class PinKind
{
    Output,
    Input
};
enum class NodeType
{
    None,
    Root,
    Sequence,
    Selector,
    Action
};
enum class BuildOpType
{
    OpenComposite,
    Action,
    CloseComposite
};
struct Node;
struct BuildOp
{
    BuildOpType Type;
    Node* EditorNode;
};
struct ActionClassInfo
{
    std::string Name;
    std::function<void(BehaviorTreeBuilder&, Node*, ParamsForAction&)> BuildFn;
    std::function<std::unique_ptr<ParamsForAction>()> CreateParamsFn;
    std::function<void(BehaviorTreeBuilder&, const std::string&, const YAML::Node&)> BuildFromYAML;
};
struct DecoratorClassInfo
{
    std::string Name;
    std::function<void(BehaviorTreeBuilder&, ParamsForDecorator&)> BuildFn;
    std::function<std::unique_ptr<ParamsForDecorator>()> CreateParamsFn;
    std::function<void(BehaviorTreeBuilder&, const std::string&, const YAML::Node&)> BuildFromYAML;
};
struct ConditionClassInfo
{
    std::string Name;
    std::function<void(BehaviorTreeBuilder&, ParamsForCondition&)> BuildFn;
    std::function<std::unique_ptr<ParamsForCondition>()> CreateParamsFn;
    std::function<void(BehaviorTreeBuilder&, const std::string&, const YAML::Node&, PriorityType)> BuildFromYAML;
};
struct BlackboardClassInfo
{
    std::string Name;
    std::function<std::unique_ptr<HBlackboard>()> CreateBlackboardFn;
};
struct EditorDecorator
{
    std::string Name;
    std::string ClassName;
    ParamsForDecorator* Params;
    EditorDecorator(const std::string& name) : Name(name), Params(nullptr) {}
};
struct EditorCondition
{
    std::string Name;
    std::string ClassName;
    ParamsForCondition* Params;
    EditorCondition(const std::string& name) : Name(name), Params(nullptr) {}
};



struct Pin
{
    nodeEditor::PinId ID;
    Node* Node;
    std::string Name;
    PinKind Kind;

    Pin(int id, const char* name):
        ID(id), Node(nullptr), Name(name), Kind(PinKind::Input)
    {
    }
};
struct Node
{
    NodeType Type;
    std::vector<EditorDecorator> Decorators;
    std::vector<EditorCondition> Conditions;
    nodeEditor::NodeId ID;
    std::string Name;
    std::vector<Pin> Inputs;
    std::vector<Pin> Outputs;
    ImColor Color;
    ImVec2 Size;

    std::string State;
    std::string SavnodeEditorState;

    Node(NodeType type, int id, const char* name,  ImColor color = ImColor(255, 255, 255)):
        Type(type), ID(id), Name(name), Color(color), Size(0, 0)
    {
    }
};
struct Link
{
    nodeEditor::LinkId ID;

    nodeEditor::PinId StartPinID;
    nodeEditor::PinId EndPinID;

    ImColor Color;

    Link(nodeEditor::LinkId id, nodeEditor::PinId startPinId, nodeEditor::PinId endPinId):
        ID(id), StartPinID(startPinId), EndPinID(endPinId), Color(255, 255, 255)
    {
    }
};
struct NodeIdLess
{
    bool operator()(const nodeEditor::NodeId& lhs, const nodeEditor::NodeId& rhs) const
    {
        return lhs.AsPointer() < rhs.AsPointer();
    }
};
