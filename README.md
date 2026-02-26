# HRealEngine

[![Language](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)](https://isocpp.org/)
[![Graphics](https://img.shields.io/badge/Graphics-OpenGL-green.svg)](https://www.opengl.org/)
[![Scripting](https://img.shields.io/badge/Scripting-C%23%20(Mono)-purple.svg)](https://www.mono-project.com/)
[![Physics](https://img.shields.io/badge/3D%20Physics-Jolt-orange.svg)](https://github.com/jrouwe/JoltPhysics)
[![Physics](https://img.shields.io/badge/2D%20Physics-Box2D-red.svg)](https://github.com/erincatto/box2d)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

**HRealEngine** is a custom game engine written in C++ with OpenGL rendering, 2D and 3D physics, and C# scripting. The goal of the engine is to build a strong AI gameplay framework with systems like **Behavior Trees**, **Perception Systems**, and **Navigation Meshes**. It currently features a built in **Behavior Tree** framework with a visual editor, an **AI Perception System** with **sight** and **hearing** senses. Navigation Meshes are actively in development.

## 🚀 Getting Started

```bash
# Clone with all submodules
git clone --recursive https://github.com/haktan313/HRealEngine

# If you already cloned without --recursive
git submodule update --init

# Generate Visual Studio project files
scripts/Win-GenProjects.bat
```

> This will invoke **Premake5** and create the required `.sln` files.

---

## 📑 Table of Contents

- [Screenshots](#-screenshots)
- [Video Demos](#-video-demos)
- [Features](#-features)
  - [Editor & Workflow](#editor--workflow)
  - [Scripting](#scripting)
  - [Behavior Tree System](#behavior-tree-system)
  - [AI Perception System](#ai-perception-system)
  - [Physics](#physics)
  - [Rendering](#rendering)
  - [Asset System](#asset-system)
  - [Project System](#project-system)
  - [Core](#core)
- [Behavior Tree Integration](#-behavior-tree-integration)
- [Behavior Tree Usage (C#)](#-behavior-tree-usage-c)
- [AI Perception System Integration and Usage](#-ai-perception-system-integration-and-usage)
- [Project Setup (C# Workflow)](#-project-setup-c-workflow)
- [Code Examples](#-code-examples)
- [Roadmap](#-roadmap)
- [Third-Party Submodules](#-third-party-submodules)

---

## 📸 Screenshots

| Behavior Tree and Percaption | Behavior Tree and Perception Runtime Debug |
|---|---|
| <img width="2556" height="1387" alt="BehaviorTreeAndPercaptionDebug" src="https://github.com/user-attachments/assets/5534108e-5f0b-43bf-b9b6-a3a15e173d04" /> | <img width="2550" height="1385" alt="BehaviorTreeRuntimeDebugAndPercaptionDebug" src="https://github.com/user-attachments/assets/2bd6cd7a-eb9d-4f72-af0b-4d08c8600827" /> |
| **3D Rendering (FBX, GLB, OBJ and Cube Batching)** | **2D Rendering** |
| <img width="2544" height="1385" alt="3DRenderonMesh" src="https://github.com/user-attachments/assets/1d8d85db-2a45-4a9e-930f-6655e3dbbc12" /> | <img width="2556" height="1394" alt="2DRendering" src="https://github.com/user-attachments/assets/db475d51-2655-407c-b307-56dc82e944f6" /> |
| **Lighting** | **Project UI** |
| <img width="2552" height="1385" alt="Lighting" src="https://github.com/user-attachments/assets/2a6c5d52-2fd5-4770-990a-511ac1253682" /> | <img width="2057" height="1194" alt="Project UI" src="https://github.com/user-attachments/assets/7301902b-b582-48a2-bcca-8b1666992f95" /> |

## 🎬 Video Demos

https://github.com/user-attachments/assets/72026fa1-6948-4b36-8498-b53c4516fdb8


https://github.com/user-attachments/assets/e94239b3-2a18-4912-888e-6bcaaf38cf1a


https://github.com/user-attachments/assets/48c53b9b-6a1b-4a75-bd9f-0d1ec41d6af4


https://github.com/user-attachments/assets/a84249d3-d87b-4e3e-9e0d-67815e860a7f



---

## ✨ Features

### Editor & Workflow
- **ImGui** based scene editor with viewport and gizmos
- Content Browser with file dialogs
- Scene serialization with **YAML**
- Entity Component System (**entt**)
- Behavior Tree tooling (visual editor + runtime debug view)

### Scripting
- **Dual scripting** support:
  - **C++ native** scripts (engine side)
  - **C# scripting** with Mono (hot reloadable assemblies)
- Script lifecycle events for entities (C#):
  - `BeginPlay`, `Tick`, `OnDestroy`
  - `OnOverlapBegin`, `OnOverlapEnd`
  - `OnEntityPerceived`, `OnEntityLost`, `OnEntityForgotten` (AI Perception callbacks)
  - Entity lifetime utilities (`Destroy`, etc.)
- Behavior Tree support in C#. Create custom Action, Decorator, Condition nodes and Blackboards
- Additional C# capabilities:
  - Open scenes at runtime
  - Spawn entities and add components dynamically
  - **RayCast** queries
  - Store, get, and persist data across levels with **GameModeData**
- **TagComponent**: supports multiple tags per entity with C# accessor functions

### Behavior Tree System
- Dockable **Behavior Tree Editor** window inside the engine
- **BehaviorTreeComponent** to assign trees to entities
- Asset Manager integration with drag & drop support
- Custom node creation in both **C++** and **C#**
- Runtime debug visualization
- See [Behavior Tree Integration](#-behavior-tree-integration) for details

### AI Perception System
- Unreal like **AI perception** framework built on top of **Jolt Physics**
- **AIControllerComponent** with configurable **sight** and **hearing** senses
- **PerceivableComponent** to mark entities as detectable targets
- **Sight sense**: Field of view angle, sight radius, line of sight raycasts with detectable point offsets
- **Hearing sense**: Noise event system with loudness, range, and attenuation
- Dedicated physics layers **(PERCEPTION / PERCEIVABLE)** isolated from gameplay physics
- Throttled updates with configurable **UpdateInterval** per AI controller
- Perception lifecycle callbacks exposed to C# scripts:
  - **OnEntityPerceived** — first detection (with perception method and position)
  - **OnEntityLost** — target leaves perception range
  - **OnEntityForgotten** — target fully forgotten after ForgetDuration
- Type filtering: Both senses support filtering by **PerceivableType** (Player, Enemy, Neutral, Environment)
- Perception query API in C#: `GetCurrentPerceptions()`, `GetForgottenPerceptions()`, `IsEntityPerceived()`, `IsEntityForgotten()`

### Physics
- **2D Physics (Box2D)**
  - `Rigidbody2DComponent` and collider components integrated into the ECS
  - Collision and overlap events forwarded to scripts
- **3D Physics (Jolt Physics)**
  - Separation between `Rigidbody3DComponent` and `Collider` components
  - Supports **Static**, **Dynamic**, and **Kinematic** body types
  - **Lock position and rotation** constraints per axis
  - **isTrigger** option for overlap only colliders
  - Collision and overlap events forwarded to both C++ and C# scripts
  - Debug collision visualization

### Rendering
- **OpenGL** rendering pipeline
- Framebuffer + RenderCommand / Renderer abstractions
- Orthographic and Perspective cameras with controllers
- **2D Renderer**
  - Batch renderer with textures and shaders
- **3D Rendering**
  - `MeshComponent` for rendering 3D entities with mesh and material assets
  - Configurable **pivot point** per mesh
  - `OBJ`, `FBX`, `GLB` mesh import through asset pipeline
  - Material system: **Albedo**, **Normal maps**, **Specular maps**
  - Debug views for normals and UVs
- **Text Rendering**
  - `TextComponent` with C# API for runtime text manipulation

### Asset System
- Centralized asset registry with asset handles
- Import pipeline for meshes, materials, scenes, textures, and Behavior Trees
- 3D asset importing using **Assimp** (OBJ, FBX, GLB/GLTF)
- Imported assets are converted into engine-native formats

### Project System
- Dedicated **Project Browser** with startup screen
- Create new C# projects from templates
- Open existing projects
- Integrated with the Mono scripting workflow (hot reloadable assemblies)
- Default scene selection on project creation
- **Build project** to standalone `.exe`

### Core
- Input handling and event system
- Logging with **spdlog**
- **TagComponent** with multi tag support and C# bindings

---

## 

## 🌲 Behavior Tree Integration

> The integration is complete. As an ongoing project, there may still be minor bugs or edge cases.

- Open the editor from the menu: **Window → Behavior Tree Editor**
- Assign trees to entities with **BehaviorTreeComponent**, trees start running when runtime begins
- Behavior Tree assets are recognized by the **Asset Manager**, drag & drop from Content Browser
- **Create** new trees: **AI System → Create Behavior Tree** (`.btree` files)
- **Import** existing trees: **AI System → Load Behavior Tree As An Asset**
- **Custom nodes (C++)**: Register with `NodeRegistry` functions inside `RegisterBehaviorTreeStuffs()` in both `EditorLayer.cpp` and `RuntimeLayer.cpp`. Example:
  ```cpp
  // Example registration calls
  NodeRegistry::AddBlackBoardToEditor<MyBlackboard>("My Blackboard");
  NodeRegistry::AddActionNodeToBuilder<MyAction, MyActionParams>("My Action");
  NodeRegistry::AddConditionNodeToBuilder<MyCondition, MyConditionParams>("My Condition");
  NodeRegistry::AddDecoratorNodeToBuilder<MyDecorator, MyDecoratorParams>("My Decorator");
  ```
- **Custom nodes (C#)**: Just create your BT classes in your C# game project, the engine automatically discovers them at initialization. However, the `RegisterBehaviorTreeStuffs()` function must be called in both `EditorLayer` and `RuntimeLayer` to register the discovered C# classes into `CSharpNodeRegistry`
- For more details, see the [BehaviorTreeLibrary documentation](https://github.com/haktan313/BehaviorTreeLibrary)

---

## 🎮 Behavior Tree Usage (C#)

### Blackboard
Derive from `BTBlackboard` and use `Create*` functions in the constructor to define blackboard values.

### Action Node
Derive from `BTActionNode` and create a parameter class from `BTActionParams`. Use `BTBlackboardKey` attribute for blackboard key references and `BTParameter` attribute for regular parameters.

### Decorator
Derive from `BTDecorator` and create a parameter class from `BTDecoratorParams`. Override `CanExecute()` to control child execution and `OnFinishedResult()` to modify the child's result status.

### Condition
Derive from `BTCondition` and create a parameter class from `BTConditionParams`. Override `CheckCondition()` to evaluate whether the condition passes.

> See [Code Examples](#-code-examples) below for full implementation samples.

---

## 👁 AI Perception System Integration and Usage
> The perception system provides Unreal like AI sensing capabilities. AI entities can detect targets through sight (FOV + line of sight) and hearing (noise events).

### Setup
1. Add an **AIControllerComponent** to your AI entity
2. Enable desired senses (Sight, Hearing) and configure their parameters
3. Add a **PerceivableComponent** to any entity that should be detectable
4. Set the perceivable entity's Type (Player, Enemy, Neutral, Environment) and optionally add **DetectablePointOffsets** for more accurate line of sight checks

### How It Works
- **Sight**: The system creates sphere trigger bodies on the `PERCEPTION` physics layer. When a `PERCEIVABLE` entity overlaps this sphere, the AI checks FOV angle (dot product) and performs line of sight raycasts. If the center raycast is blocked, fallback raycasts are attempted against **DetectablePointOffsets** on the target.
- **Hearing**: Similar to **sight**, a sphere trigger body is created on the `PERCEPTION` layer. Only noise events from entities already overlapping this sphere are processed. The system then applies type filtering, distance based loudness attenuation, and adds qualifying sources to **CurrentPerceptions**.
- **Lifecycle**: Each AI controller ticks at its own **UpdateInterval**. Every tick, it builds a fresh **CurrentPerceptions** list and diffs against the previous tick to fire `OnEntityPerceived`, `OnEntityLost`, and `OnEntityForgotten` callbacks.

### C# Callbacks
- Override these virtual methods on your Entity script to respond to perception events:
```csharp
    public class EnemyAI : Entity
    {
        public override void OnEntityPerceived(ulong entityID, int perceptionMethod, Vector3 position)
        {
            // perceptionMethod: 0 = Sight, 1 = Hearing
            Console.WriteLine($"Detected entity {entityID} via {(perceptionMethod == 0 ? "Sight" : "Hearing")} at {position}");
        }

        public override void OnEntityLost(ulong entityID, Vector3 lastKnownPosition)
        {
            Console.WriteLine($"Lost sight of entity {entityID}, last seen at {lastKnownPosition}");
        }

        public override void OnEntityForgotten(ulong entityID)
        {
            Console.WriteLine($"Completely forgot about entity {entityID}");
        }
    }
```
### Reporting Noise C#
```csharp
// Emit a noise event at the current entity's position
Entity.ReportNoise(
    position: transform.Position,
    loudness: 1.0f,
    maxRange: 15.0f,
    sourceType: PerceivableType.Player);
```
---

## 🛠 Project Setup (C# Workflow)

### 1. Create a New Project
1. Launch **HRealEngine Editor**, the Project Browser appears on startup
2. Select **New Project (C#)**
3. Choose a project location and name, a `.hprj` file will be created
4. The editor opens with a default scene and project layout

### 2. Generate the C# Script Project
5. Navigate to the project's `Scripts` directory
6. Run `Win-GenProject.bat`, this invokes Premake and generates the `.sln` and `.csproj` files
7. Open the solution in **Visual Studio** or **JetBrains Rider**

### 3. Build and Run Scripts
8. Build the C# solution, the assembly (`.dll`) and debug symbols (`.pdb`) will be generated under `Scripts/Binaries/`
9. Restart the editor or reload the project, the engine automatically detects and loads the compiled assembly

---

## 📝 Code Examples

<details>
<summary><b>Sight Percaption Update</b></summary>

```cpp
    void JoltWorld::SightPercaptionsUpdate(AIControllerComponent& ai, const TransformComponent& tc,
        const glm::vec3& forward)
    {
        for (const UUID& targetID : ai.OverlappingEntities)
        {
            Entity targetEntity = m_Scene->GetEntityByUUID(targetID);
            if (!targetEntity || !targetEntity.HasComponent<PerceivableComponent>())
                continue;       
            
            auto& percComp = targetEntity.GetComponent<PerceivableComponent>();
            if (!percComp.bIsDetectable)
                continue;       
            
            auto& targetTC = targetEntity.GetComponent<TransformComponent>();
            glm::vec3 toTarget = targetTC.Position - tc.Position;
            float distance = glm::length(toTarget);
            if (distance < 0.001f)
                continue;       
            
            glm::vec3 dirToTarget = toTarget / distance;        
            if (ai.IsSightEnabled() && distance <= ai.SightSettings.SightRadius)
            {
                bool typeMatch = ai.SightSettings.DetectableTypes.empty();
                for (auto& dt : ai.SightSettings.DetectableTypes)
                    for (auto& pt : percComp.Types)
                        if (dt == pt)
                        {
                            typeMatch = true;
                            break;
                        }
                
                if (!typeMatch)
                    continue;       
                
                float dotProduct = glm::dot(glm::normalize(forward), dirToTarget);
                float halfFOVcos = glm::cos(glm::radians(ai.SightSettings.FieldOfView * 0.5f));
                if (dotProduct < halfFOVcos)
                    continue;       
                
                std::vector<uint64_t> ignoreList = { (uint64_t)ai.OwnerEntityID };
                RaycastHit3D losHit = Raycast(tc.Position, dirToTarget, distance + 0.1f, false, 0.0f, ignoreList);      
                bool hasLOS = false;
                
                if (losHit.Hit && losHit.HitEntityID == targetID)
                    hasLOS = true;
                else if (!percComp.DetectablePointsOffsets.empty())
                    for (auto& offset : percComp.DetectablePointsOffsets)
                    {
                        glm::vec3 targetPoint = targetTC.Position + offset;
                        glm::vec3 dir = glm::normalize(targetPoint - tc.Position);
                        float dist = glm::length(targetPoint - tc.Position);
                        RaycastHit3D pointHit = Raycast(tc.Position, dir, dist + 0.1f, false, 0.0f, ignoreList);
                        if (pointHit.Hit && pointHit.HitEntityID == targetID)
                        {
                            hasLOS = true;
                            break;
                        }
                    }       
                
                if (hasLOS)
                {
                    PercaptionResult result;
                    result.EntityID.ID = targetID;
                    result.Type = percComp.Types.empty() ? PerceivableType::Neutral : percComp.Types[0];
                    result.PercaptionMethod = PercaptionType::Sight;
                    result.SensedPosition = targetTC.Position;
                    result.TimeSinceLastSensed = 0.0f;
                    ai.CurrentPerceptions.push_back(result);
                }
            }
        }
    }
```
</details>

<details>
<summary><b>Blackboard (C#)</b></summary>

```csharp
using HRealEngine.BehaviorTree;

public class ExampleAIBlackboard : BTBlackboard
{
    public ExampleAIBlackboard()
    {
        CreateBool("IsPlayerInRange", false);
        CreateFloat("Health", 100.0f);
        CreateString("PlayerTag", "Player");
        CreateInt("AmmoCount", 10);
    }
}
```
</details>

<details>
<summary><b>Action Node (C#)</b></summary>

```csharp
using System;
using HRealEngine.BehaviorTree;

namespace HRealEngine
{
    public class ExampleActionParams : BTActionParams
    {
        [BTParameter("Example Parameter")]
        public string exampleParameter = "Default Value";
        [BTParameter("Example Number")]
        public int exampleNumber = 42;
        [BTParameter("Example Flag")]
        public bool exampleFlag = true;
        [BTBlackboardKey(BTBlackboardKeyAttribute.KeyType.String, "ExampleBlackboardKey")]
        public string exampleBlackboardKey = "ExampleKey";
        [BTBlackboardKey(BTBlackboardKeyAttribute.KeyType.Float, "ExampleFloatKey")]
        public string exampleFloatBlackboardKey = "ExampleFloatKey";
    }

    public class ExampleAction : BTActionNode
    {
        private ExampleActionParams actionParams;

        public ExampleAction()
        {
            actionParams = new ExampleActionParams();
            SetParameters(actionParams);
        }

        public override void SetParameters(BTActionParams param)
        {
            base.SetParameters(param);
            actionParams = param as ExampleActionParams;
        }

        protected override void OnInitialize()
        {
            actionParams = parameters as ExampleActionParams;
        }

        public override void OnStart()
        {
            Console.WriteLine("ExampleAction: Starting action with parameters:");
            Console.WriteLine($"Example Parameter: {actionParams.exampleParameter}");
            Console.WriteLine($"Example Number: {actionParams.exampleNumber}");
            Console.WriteLine($"Example Flag: {actionParams.exampleFlag}");

            blackboard.SetString(actionParams.exampleBlackboardKey, actionParams.exampleParameter);
            string blackboardValue = blackboard.GetFloat(actionParams.exampleFloatBlackboardKey).ToString();
            Console.WriteLine($"Example Blackboard Value for '{actionParams.exampleFloatBlackboardKey}': {blackboardValue}");
        }

        public override NodeStatus Update()
        {
            return NodeStatus.Success;
        }

        public override void OnFinished()
        {
            Console.WriteLine("ExampleAction: Finished action.");
        }
    }
}
```
</details>

<details>
<summary><b>Decorator (C#)</b></summary>

```csharp
using System;
using HRealEngine.BehaviorTree;

namespace HRealEngine
{
    public class ExampleDecoratorParams : BTDecoratorParams
    {
        [BTParameter("Example Decorator Parameter")]
        public string exampleDecoratorParameter = "Default Decorator Value";
        [BTBlackboardKey(BTBlackboardKeyAttribute.KeyType.String, "ExampleDecoratorBlackboardKey")]
        public string exampleDecoratorBlackboardKey = "ExampleDecoratorKey";
    }

    public class ExampleDecorator : BTDecorator
    {
        private ExampleDecoratorParams decoratorParams;

        public ExampleDecorator()
        {
            decoratorParams = new ExampleDecoratorParams();
            SetParameters(decoratorParams);
        }

        protected override void OnInitialize()
        {
            decoratorParams = parameters as ExampleDecoratorParams;
        }

        public override void SetParameters(BTDecoratorParams param)
        {
            base.SetParameters(param);
            decoratorParams = param as ExampleDecoratorParams;
        }

        public override bool CanExecute()
        {
            string blackboardValue = blackboard.GetString(decoratorParams.exampleDecoratorBlackboardKey);
            Console.WriteLine($"ExampleDecorator: Checking CanExecute with Blackboard Value for '{decoratorParams.exampleDecoratorBlackboardKey}': {blackboardValue}");
            return blackboardValue == decoratorParams.exampleDecoratorParameter;
        }

        public override void OnFinishedResult(ref NodeStatus status)
        {
            Console.WriteLine($"ExampleDecorator: Finished with status: {status}");
        }

        public override void OnFinished()
        {
            Console.WriteLine("ExampleDecorator: Finished execution.");
        }
    }
}
```
</details>

<details>
<summary><b>Condition (C#)</b></summary>

```csharp
using System;
using HRealEngine.BehaviorTree;

namespace HRealEngine
{
    public class ExampleConditionParams : BTConditionParams
    {
        [BTParameter("Example Condition Parameter")]
        public string exampleConditionParameter = "Default Condition Value";
        [BTBlackboardKey(BTBlackboardKeyAttribute.KeyType.String, "ExampleConditionBlackboardKey")]
        public string exampleConditionBlackboardKey = "ExampleConditionKey";
    }

    public class ExampleCondition : BTCondition
    {
        private ExampleConditionParams conditionParams;

        public ExampleCondition()
        {
            conditionParams = new ExampleConditionParams();
            SetParameters(conditionParams);
        }

        protected override void OnInitialize()
        {
            conditionParams = parameters as ExampleConditionParams;
        }

        public override void SetParameters(BTConditionParams param)
        {
            base.SetParameters(param);
            conditionParams = param as ExampleConditionParams;
        }

        public override bool CheckCondition()
        {
            string blackboardValue = blackboard.GetString(conditionParams.exampleConditionBlackboardKey);
            Console.WriteLine($"ExampleCondition: Checking condition with Blackboard Value for '{conditionParams.exampleConditionBlackboardKey}': {blackboardValue}");
            return blackboardValue == conditionParams.exampleConditionParameter;
        }

        public override void OnFinished()
        {
            Console.WriteLine("ExampleCondition: Finished checking condition.");
        }
    }
}
```
</details>

<details>
<summary><b>BT Integration in Scene.cpp (C++)</b></summary>

```cpp
void Scene::StartBTs()
{
    Root::RootClear();
    auto view = m_Registry.view<BehaviorTreeComponent>();
    for (auto e : view)
    {
        Entity entity = {e, this};
        auto& btComponent = entity.GetComponent<BehaviorTreeComponent>();
        if (btComponent.BehaviorTreeAsset)
        {
            if (m_BehaviorTreeCache.find(btComponent.BehaviorTreeAsset) == m_BehaviorTreeCache.end())
            {
                auto metaData = Project::GetActive()->GetEditorAssetManager()->GetAssetMetadata(btComponent.BehaviorTreeAsset);
                auto path = Project::GetAssetDirectory() / metaData.FilePath;
                auto name = metaData.FilePath.stem().string();

                YAML::Node data = YAML::LoadFile(path.string());

                BehaviorTree* bt = Root::CreateBehaviorTree(name, path.string());
                BTSerializer serializer(bt);
                serializer.Deserialize(data);
                m_BehaviorTreeCache[btComponent.BehaviorTreeAsset] = data;

                UUID ownerUUID = entity.GetUUID();
                m_BTOwnerUUIDs[btComponent.BehaviorTreeAsset] = ownerUUID;
                bt->SetOwner<UUID>(&m_BTOwnerUUIDs[btComponent.BehaviorTreeAsset]);
                bt->StartTree();
            }
            else
            {
                YAML::Node& data = m_BehaviorTreeCache.at(btComponent.BehaviorTreeAsset);
                auto metaData = Project::GetActive()->GetEditorAssetManager()->GetAssetMetadata(btComponent.BehaviorTreeAsset);
                auto path = Project::GetAssetDirectory() / metaData.FilePath;
                auto name = metaData.FilePath.stem().string();

                BehaviorTree* bt = Root::CreateBehaviorTree(name, path.string());
                BTSerializer serializer(bt);
                serializer.Deserialize(data);

                UUID ownerUUID = entity.GetUUID();
                m_BTOwnerUUIDs[btComponent.BehaviorTreeAsset] = ownerUUID;
                bt->SetOwner<UUID>(&m_BTOwnerUUIDs[btComponent.BehaviorTreeAsset]);
                bt->StartTree();
            }
        }
    }
}

void Scene::StopBTs()
{
    Root::RootClear();
    m_BehaviorTreeCache.clear();
    m_BTOwnerUUIDs.clear();
}
```
</details>

<details>
<summary><b>Overlap / Collision Events (C++)</b></summary>

```cpp
// --- Jolt Physics 3D Collision Events ---
void JoltWorld::UpdateRuntime3D()
{
    std::vector<CollisionEvent> beginEvents, endEvents;
    {
        std::lock_guard<std::mutex> lock(m_EventQueueMutex);
        beginEvents = std::move(m_CollisionBeginEvents);
        endEvents = std::move(m_CollisionEndEvents);

        m_CollisionBeginEvents.clear();
        m_CollisionEndEvents.clear();
    }

    for (const auto& ev : beginEvents)
    {
        Entity a = m_Scene->GetEntityByUUID(ev.EntityA);
        Entity b = m_Scene->GetEntityByUUID(ev.EntityB);

        if (a && b)
        {
            if (m_Scene->GetRegistry().any_of<ScriptComponent>(a))
                ScriptEngine::OnCollisionBegin(a, b);

            if (m_Scene->GetRegistry().any_of<ScriptComponent>(b))
                ScriptEngine::OnCollisionBegin(b, a);

            if (a.HasComponent<NativeScriptComponent>())
            {
                auto& nsc = a.GetComponent<NativeScriptComponent>();
                if (nsc.Instance) nsc.Instance->OnCollisionBegin(b);
            }
            if (b.HasComponent<NativeScriptComponent>())
            {
                auto& nsc = b.GetComponent<NativeScriptComponent>();
                if (nsc.Instance) nsc.Instance->OnCollisionBegin(a);
            }
        }
    }

    for (const auto& ev : endEvents)
    {
        Entity a = m_Scene->GetEntityByUUID(ev.EntityA);
        Entity b = m_Scene->GetEntityByUUID(ev.EntityB);
        if (a && b)
        {
            if (m_Scene->GetRegistry().any_of<ScriptComponent>(a))
                ScriptEngine::OnCollisionEnd(a, b);

            if (m_Scene->GetRegistry().any_of<ScriptComponent>(b))
                ScriptEngine::OnCollisionEnd(b, a);

            if (a.HasComponent<NativeScriptComponent>())
            {
                auto& nsc = a.GetComponent<NativeScriptComponent>();
                if (nsc.Instance) nsc.Instance->OnCollisionEnd(b);
            }
            if (b.HasComponent<NativeScriptComponent>())
            {
                auto& nsc = b.GetComponent<NativeScriptComponent>();
                if (nsc.Instance) nsc.Instance->OnCollisionEnd(a);
            }
        }
    }
}

// --- Jolt Contact Listener ---
class MyContactListener : public JPH::ContactListener
{
public:
    MyContactListener(Scene* scene, JoltWorld* joltWorld)
        : m_Scene(scene), m_JoltWorld(joltWorld) {}

    virtual JPH::ValidateResult OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2,
        JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override
    {
        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2,
        const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
    {
        auto entity1ID = (UUID)inBody1.GetUserData();
        auto entity2ID = (UUID)inBody2.GetUserData();
        Entity entity1 = m_Scene->GetEntityByUUID(entity1ID);
        Entity entity2 = m_Scene->GetEntityByUUID(entity2ID);
        if (m_Scene->GetRegistry().valid(entity1) && m_Scene->GetRegistry().valid(entity2))
            m_JoltWorld->m_CollisionBeginEvents.push_back({ entity1, entity2 });
    }

    virtual void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2,
        const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
    {
    }

    virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override
    {
        auto body1ID = inSubShapePair.GetBody1ID();
        auto body2ID = inSubShapePair.GetBody2ID();
        const JPH::BodyInterface& bi = m_JoltWorld->physics_system.GetBodyInterfaceNoLock();

        UUID entity1ID = (UUID)bi.GetUserData(body1ID);
        UUID entity2ID = (UUID)bi.GetUserData(body2ID);
        if (entity1ID == 0 || entity2ID == 0)
            return;

        Entity entity1 = m_Scene->GetEntityByUUID(entity1ID);
        Entity entity2 = m_Scene->GetEntityByUUID(entity2ID);
        if (m_Scene->GetRegistry().valid(entity1) && m_Scene->GetRegistry().valid(entity2))
            m_JoltWorld->m_CollisionEndEvents.push_back({ entity1, entity2 });
    }

private:
    Scene* m_Scene = nullptr;
    JoltWorld* m_JoltWorld = nullptr;
};

// --- Box2D 2D Physics Setup ---
void Scene::OnPhysics2DStart()
{
    m_PhysicsWorld = new b2World({0.0f, -9.81f});
    m_PhysicsWorld->SetContactListener(new GameContactListener(this));

    auto view = m_Registry.view<Rigidbody2DComponent>();
    for (auto e : view)
    {
        Entity entity = {e, this};
        auto& transform = entity.GetComponent<TransformComponent>();
        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

        b2BodyDef bodyDef;
        bodyDef.type = RigidBody2DTypeToBox2D(rb2d.Type);
        bodyDef.position.Set(transform.Position.x, transform.Position.y);
        bodyDef.angle = transform.Rotation.z;

        b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
        body->SetFixedRotation(rb2d.FixedRotation);
        body->GetUserData().pointer = entity.GetUUID();
        rb2d.RuntimeBody = body;

        if (entity.HasComponent<BoxCollider2DComponent>())
        {
            auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
            b2PolygonShape shape;
            shape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &shape;
            fixtureDef.density = bc2d.Density;
            fixtureDef.friction = bc2d.Friction;
            fixtureDef.restitution = bc2d.Restitution;
            fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
            body->CreateFixture(&fixtureDef);
        }

        if (entity.HasComponent<CircleCollider2DComponent>())
        {
            auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
            b2CircleShape shape;
            shape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
            shape.m_radius = cc2d.Radius * transform.Scale.x;

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &shape;
            fixtureDef.density = cc2d.Density;
            fixtureDef.friction = cc2d.Friction;
            fixtureDef.restitution = cc2d.Restitution;
            fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;
            body->CreateFixture(&fixtureDef);
        }
    }
}

// --- Box2D Contact Listener ---
void Scene::GameContactListener::BeginContact(b2Contact* contact)
{
    b2Body* bodyA = contact->GetFixtureA()->GetBody();
    b2Body* bodyB = contact->GetFixtureB()->GetBody();

    auto entityA = m_Scene->GetEntityByUUID((UUID)bodyA->GetUserData().pointer);
    auto entityB = m_Scene->GetEntityByUUID((UUID)bodyB->GetUserData().pointer);
    if (m_Scene->m_Registry.valid(entityA) && m_Scene->m_Registry.valid(entityB))
        m_Scene->m_CollisionBeginEvents.push_back({ entityA, entityB });
}

void Scene::GameContactListener::EndContact(b2Contact* contact)
{
    b2Body* bodyA = contact->GetFixtureA()->GetBody();
    b2Body* bodyB = contact->GetFixtureB()->GetBody();

    auto entityA = m_Scene->GetEntityByUUID((UUID)bodyA->GetUserData().pointer);
    auto entityB = m_Scene->GetEntityByUUID((UUID)bodyB->GetUserData().pointer);
    if (m_Scene->m_Registry.valid(entityA) && m_Scene->m_Registry.valid(entityB))
        m_Scene->m_CollisionEndEvents.push_back({ entityA, entityB });
}
```
</details>

---

## 🗺 Roadmap

- [ ] Custom Navigation Mesh library
- [ ] Animation system

---

## 📦 Third-Party Submodules

HRealEngine uses the following libraries (all included as Git submodules):

| Library | Description |
|---|---|
| [Jolt Physics](https://github.com/jrouwe/JoltPhysics) | 3D physics engine |
| [Box2D](https://github.com/erincatto/box2d) | 2D physics engine |
| [Mono](https://github.com/mono/mono) | C# scripting runtime |
| [entt](https://github.com/skypjack/entt) | Entity Component System |
| [GLFW](https://github.com/glfw/glfw) | Window and input handling |
| [Glad](https://github.com/Dav1dde/glad) | OpenGL function loader |
| [ImGui](https://github.com/ocornut/imgui) | Immediate mode GUI |
| [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) | Gizmo manipulation for ImGui |
| [spdlog](https://github.com/gabime/spdlog) | Fast logging library |
| [stb_image](https://github.com/nothings/stb) | Image loading |
| [yaml-cpp](https://github.com/jbeder/yaml-cpp) | YAML serialization |
| [glm](https://github.com/g-truc/glm) | Math library for graphics |
| [Assimp](https://github.com/assimp/assimp) | Asset importing (OBJ/FBX/GLB/GLTF) |
| [FileWatch](https://github.com/ThomasMonkman/filewatch) | File system watcher |
