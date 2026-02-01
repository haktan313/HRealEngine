# HRealEngine

**HRealEngine** is a custom **Game Engine** written in C++ built with **OpenGL**. It started as a **2D focused engine**, and has been expanding toward **3D rendering**, **3D physics** and **AI driven** gameplay systems. 
The primary long term goal of the engine is to serve as a playground for AI frameworks, including **Behavior Trees**, **Perception systems**, and **Navigation Meshes**, while remaining a fully functional general purpose engine.

- Start with cloning the repo with `git clone --recursive https://github.com/haktan313/HRealEngine`

- If you cloned it without the `--recursive` flag, initialize submodules manually `git submodule update --init`

- If you make changes to the build files or need to regenerate Visual Studio project files, run `scripts/Win-GenProjects.bat` this will call **premake5** and create the required `.sln` files.
  
## Table of Contents

- [HRealEngine](#hrealengine)
  - [BehaviorTreeLibrary Integration](#behaviortreelibrary-integration)
  - [Screenshots](#-screenshots)
  - [Editor & Workflow](#editor--workflow)
  - [Scripting](#scripting)
  - [Physics](#physics)
  - [Rendering](#rendering)
  - [Asset System](#asset-system)
  - [Project System](#project-system)
  - [Core](#core)
  - [Project Setup (C# Workflow)](#project-setup-c-workflow)
  - [Notes / Roadmap](#notes--roadmap)
  - [Script Example (BT integration in Scene.cpp)](#script-example-bt-integration-in-scenecpp)
  - [Script Example (Overlap Events)](#script-example-overlap-events)
  - [Third-Party Submodules](#-third-party-submodules)

### BehaviorTreeLibrary Integration
> The integration is completed, but as an ongoing project there may still be minor bugs or missing edge cases.

- A **dockable Behavior Tree Editor** window is available inside the engine.
  - Open it from the editor menu: **Window → Behavior Tree Editor**.
- A **BehaviorTreeComponent** lets you assign behavior trees to entities.
  - Behavior Trees currently start running when **runtime begins**.
- Behavior Tree assets are recognized by the **Asset Manager**.
  - You can **drag & drop** a Behavior Tree asset from the Content Browser into the component.
- Creating / importing Behavior Tree assets:
  - Use **AI System → Create Behavior Tree** to create a new `.btree` asset.
  - Use **AI System → Load Behavior Tree As An Asset** to import an existing behavior tree file into the project as an engine asset.
- Custom nodes & blackboards:
  - To register custom **Blackboards / Actions / Conditions / Decorators**, call `NodeRegistry` registration functions in EditorLayer.cpp and inside of the `RegisterBehaviorTreeStuffs()`.
  - For more details and examples, check the **BehaviorTreeLibrary** documentation **https://github.com/haktan313/BehaviorTreeLibrary**

https://github.com/user-attachments/assets/4c010b09-0b99-43da-9000-b52fa8ad2800

https://github.com/user-attachments/assets/07c87738-2202-4d6c-8949-1bdc5601b3f6

https://github.com/user-attachments/assets/8d94ccb6-fb48-44e6-83d7-c2b3afed06c8

https://github.com/user-attachments/assets/47cb6072-ca52-478f-9e62-6ded3dbc4675

## 📸 Screenshots
| Behavior Tree | Behavior Tree Runtime Debug |
|--------|-------------|
| <img width="1911" height="1025" alt="Screenshot 2026-01-19 144030" src="https://github.com/user-attachments/assets/e0bd364a-96fc-4e4e-948b-18770dc65a2d" /> | <img width="1904" height="1025" alt="Screenshot 2026-01-19 143837" src="https://github.com/user-attachments/assets/91a8c784-79d0-4525-895d-98a4ed19e446" /> |
| 3D | OBJ Mesh |
| <img width="1913" height="1137" alt="image" src="https://github.com/user-attachments/assets/8a9c1386-550d-4038-b17b-8183d732b641" /> | <img width="1894" height="1141" alt="Screenshot 2026-01-02 161346" src="https://github.com/user-attachments/assets/4f5b43d3-7c31-4e98-a4a6-6c5ec470f71d" /> |
| 2D | Asset Registry |
| <img width="2555" height="1386" alt="image" src="https://github.com/user-attachments/assets/821f02a4-56d2-4494-86b3-265eb5e2189d" /> | <img width="1324" height="937" alt="Screenshot 2026-01-02 161443" src="https://github.com/user-attachments/assets/61ad632f-7c6c-4ee9-8fea-8b7d2a2a99f6" /> |
| Lighting | Project UI |
| <img width="2552" height="1385" alt="Screenshot 2026-02-01 174512" src="https://github.com/user-attachments/assets/2a6c5d52-2fd5-4770-990a-511ac1253682" /> | <img width="2057" height="1194" alt="Screenshot 2026-02-01 172746" src="https://github.com/user-attachments/assets/7301902b-b582-48a2-bcca-8b1666992f95" /> |

### Editor & Workflow
- ImGui-based scene editor with viewport and gizmos
- Content Browser + file dialogs
- Scene serialization with YAML
- Entity Component System (entt)
- Behavior Tree tooling (editor + runtime debug)

### Scripting
- Dual scripting support:
  - C++ Native scripts (engine side scripting)
  - C# scripting with Mono (hot reloadable assemblies)
- Typical script events:
  - OnCreate, OnUpdate, OnDestroy, OnOverlapBegin, OnOverlapEnd, Destroy, etc.
  - Overlap begin/end events
  - Entity lifetime utilities (Destroy, etc.)

### Physics
- **2D Physics (Box2D)**
  - Rigidbody2D and collider components integrated into the ECS
  - Collision and overlap events forwarded to scripts
- **3D Physics (Jolt Physics)**
  - Initial 3D physics pipeline
  - Separation between **Rigidbody3D** and **Collider components**
  - Supports **Static**, **Dynamic**, and **Kinematic** bodies
  - Collision and overlap events forwarded to both C++ and C# scripts
  - Debug collision visualization support

### Rendering
- OpenGL rendering
- Framebuffer + RenderCommand / Renderer abstractions
- Orthographic and Perspective cameras with controllers
- **2D Renderer**
  - Batch renderer with textures and shaders
- **3D Rendering**
  - `MeshComponent` for rendering 3D entities using mesh and material assets
  - `OBJ`, `FBX`, `GLB` mesh import and asset based rendering pipeline
  - Material system supporting:
    - **Albedo textures**
    - **Normal maps**
    - **Specular maps**
  - Texture assignment support for 3D meshes
  - Debug views for normals and UVs
  - Improved lighting interaction with materials

### Asset System
- Centralized asset registry and asset handles
- Import pipeline for meshes, materials, scenes, textures, and Behavior Trees
- Asset based references used across rendering and scene systems
- Supports importing 3D assets using **Assimp**
- Supported formats include:
  - OBJ
  - FBX
  - GLB / GLTF
- Imported meshes and materials are converted into engine-native asset formats

### Project System
- Added a dedicated **Project System** with an engine startup screen.
- Users can:
  - **Create a New Project C#**
  - **Open an Existing Project**
- New projects can be created from **template projects**.
- **C#** projects are integrated with the scripting workflow (Mono + hot reloadable assemblies)
- Improved project startup flow, including default scene selection.

### Core
- Input handling and event system
- Logging with spdlog

## Project Setup (C# Workflow)
### Creating a New Project
1. Launch **HRealEngine Editor**.  
   The **Project Browser** window will appear on startup.
2. Select **New Project (C#)**.
3. Choose a project location and name.  
   A `.hprj` file will be created to represent the project.
4. The editor will open with a default scene and initial project layout.

### Generating the C# Script Project
5. Navigate to the project’s `Scripts` directory.
6. Run `Win-GenProject.bat`.  
   This will:
   - Invoke **Premake**
   - Generate a Visual Studio **C# solution**
   - Create the required `.sln` and `.csproj` files
7. Open the generated solution in **Visual Studio** or **JetBrains Rider**.

### Building Scripts
8. Build the C# solution.  
   On a successful build:
   - The script assembly (`.dll`) will be generated under  
     `Scripts/Binaries/`
   - Debug symbols (`.pdb`) will also be produced
9. Restart the editor or reload the project.  
   The engine will automatically detect and load the compiled script assembly.


## Notes / Roadmap
- **Implementing Animations**
- **Creating an Unreal like Perception System**
- **My own Navigation Mesh library implementation**

## Script Example (BT integration in Scene.cpp)

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
                    bt->SetOwner<Entity>(&entity);
                    bt->StartTree();
                }
                else
                {
                    YAML::Node& data = m_BehaviorTreeCache.at(btComponent.BehaviorTreeAsset);/*m_BehaviorTreeCache[btComponent.BehaviorTreeAsset];*/
                    auto metaData = Project::GetActive()->GetEditorAssetManager()->GetAssetMetadata(btComponent.BehaviorTreeAsset);
                    auto path = Project::GetAssetDirectory() / metaData.FilePath;
                    auto name = metaData.FilePath.stem().string();
                    
                    BehaviorTree* bt = Root::CreateBehaviorTree(name, path.string());
                    BTSerializer serializer(bt);
                    serializer.Deserialize(data);
                    bt->SetOwner<Entity>(&entity);
                    bt->StartTree();
                }
            }
        }
    }

    void Scene::StopBTs()
    {
        Root::RootClear();
        m_BehaviorTreeCache.clear();
    }
```

## Script Example (Overlap Events)

```cpp
        if (!m_CollisionBeginEvents.empty())
        {
            for (const auto& collisionEvent : m_CollisionBeginEvents)
            {
                if (m_Scene->GetRegistry().any_of<ScriptComponent>(collisionEvent.A))
                {
                    Entity entityA = {collisionEvent.A, m_Scene};
                    ScriptEngine::OnCollisionBegin(entityA, Entity{collisionEvent.B, m_Scene});
                }
                if (m_Scene->GetRegistry().any_of<ScriptComponent>(collisionEvent.B))
                {
                    Entity entityB = {collisionEvent.B, m_Scene};
                    ScriptEngine::OnCollisionBegin(entityB, Entity{collisionEvent.A, m_Scene});
                }       
                if (m_Scene->GetRegistry().any_of<NativeScriptComponent>(collisionEvent.A))
                {
                    Entity entityA = {collisionEvent.A, m_Scene};
                    auto& nsc = entityA.GetComponent<NativeScriptComponent>();
                    if (nsc.Instance)
                        nsc.Instance->OnCollisionBegin(Entity{collisionEvent.B, m_Scene});
                }
                if (m_Scene->GetRegistry().any_of<NativeScriptComponent>(collisionEvent.B))
                {
                    Entity entityB = {collisionEvent.B, m_Scene};
                    auto& nsc = entityB.GetComponent<NativeScriptComponent>();
                    if (nsc.Instance)
                        nsc.Instance->OnCollisionBegin(Entity{collisionEvent.A, m_Scene});
                }
            }
            m_CollisionBeginEvents.clear();
        }
        if (!m_CollisionEndEvents.empty())
        {
            for (const auto& collisionEvent : m_CollisionEndEvents)
            {
                if (m_Scene->GetRegistry().any_of<ScriptComponent>(collisionEvent.A))
                {
                    Entity entityA = {collisionEvent.A, m_Scene};
                    ScriptEngine::OnCollisionEnd(entityA, Entity{collisionEvent.B, m_Scene});
                }
                if (m_Scene->GetRegistry().any_of<ScriptComponent>(collisionEvent.B))
                {
                    Entity entityB = {collisionEvent.B, m_Scene};
                    ScriptEngine::OnCollisionEnd(entityB, Entity{collisionEvent.A, m_Scene});
                }       
                if (m_Scene->GetRegistry().any_of<NativeScriptComponent>(collisionEvent.A))
                {
                    Entity entityA = {collisionEvent.A, m_Scene};
                    auto& nsc = entityA.GetComponent<NativeScriptComponent>();
                    if (nsc.Instance)
                        nsc.Instance->OnCollisionEnd(Entity{collisionEvent.B, m_Scene});
                }
                if (m_Scene->GetRegistry().any_of<NativeScriptComponent>(collisionEvent.B))
                {
                    Entity entityB = {collisionEvent.B, m_Scene};
                    auto& nsc = entityB.GetComponent<NativeScriptComponent>();
                    if (nsc.Instance)
                        nsc.Instance->OnCollisionEnd(Entity{collisionEvent.A, m_Scene});
                }
            }
            m_CollisionEndEvents.clear();
        }
    -------------------Jolt
        class MyContactListener : public JPH::ContactListener
        {
        public:
            MyContactListener(Scene* scene, JoltWorld* joltWorld) : m_Scene(scene), m_JoltWorld(joltWorld) {}
            virtual JPH::ValidateResult	OnContactValidate(const JPH::Body &inBody1, const JPH::Body &inBody2,
                JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult &inCollisionResult) override
            {
                //std::cout << "Contact validate callback" << std::endl;
                // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
                return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
            }

            virtual void OnContactAdded(const JPH::Body &inBody1, const JPH::Body &inBody2,
                const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) override
            {
                auto entity1ID = (UUID)inBody1.GetUserData();
                auto entity2ID = (UUID)inBody2.GetUserData();
                Entity entity1 = m_Scene->GetEntityByUUID(entity1ID);
                Entity entity2 = m_Scene->GetEntityByUUID(entity2ID);
                if (m_Scene->GetRegistry().valid(entity1) && m_Scene->GetRegistry().valid(entity2))
                    m_JoltWorld->m_CollisionBeginEvents.push_back({ entity1,  entity2 });
                std::cout << "A contact was added" << std::endl;
            }

            virtual void OnContactPersisted(const JPH::Body &inBody1, const JPH::Body &inBody2,
                const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) override
            {
                //std::cout << "A contact was persisted" << std::endl;
            }

            virtual void OnContactRemoved(const JPH::SubShapeIDPair &inSubShapePair) override
            {
                auto body1ID = inSubShapePair.GetBody1ID();
                auto body2ID = inSubShapePair.GetBody2ID();
                const JPH::BodyInterface &bi = m_JoltWorld->physics_system.GetBodyInterfaceNoLock();

                uint64_t userData1 = bi.GetUserData(body1ID);
                uint64_t userData2 = bi.GetUserData(body2ID);

                UUID entity1ID = (UUID)userData1;
                UUID entity2ID = (UUID)userData2;
                if (entity1ID == 0 || entity2ID == 0)
                    return;
                Entity entity1 = m_Scene->GetEntityByUUID(entity1ID);
                Entity entity2 = m_Scene->GetEntityByUUID(entity2ID);
                if (m_Scene->GetRegistry().valid(entity1) && m_Scene->GetRegistry().valid(entity2))
                    m_JoltWorld->m_CollisionEndEvents.push_back({ entity1,  entity2 });
                std::cout << "A contact was removed" << std::endl;
            }
        private:
            Scene* m_Scene = nullptr;
            JoltWorld* m_JoltWorld = nullptr;
        };
---------------Box2D
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

I used a few submodules there is:

## 📦 Third-Party Submodules

HRealEngine uses the following libraries (all included as Git submodules):

- **[Jolt Physics](https://github.com/jrouwe/JoltPhysics)** - 3D physics engine
- **[Box2D](https://github.com/erincatto/box2d)** – 2D physics engine
- **[mono](https://github.com/mono/mono)** - C# scripting
- **[entt](https://github.com/skypjack/entt)** – Entity Component System
- **[GLFW](https://github.com/glfw/glfw)** – Window and input handling
- **[Glad](https://github.com/Dav1dde/glad)** – OpenGL function loader
- **[ImGui](https://github.com/ocornut/imgui)** – Immediate mode GUI
- **[ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo)** – Gizmo manipulation for ImGui
- **[spdlog](https://github.com/gabime/spdlog)** – Fast logging library
- **[stb_image](https://github.com/nothings/stb)** – Image loading
- **[yaml-cpp](https://github.com/jbeder/yaml-cpp)** – Serialization
- **[glm](https://github.com/g-truc/glm)** – Math library for graphics
- **[assimp](https://github.com/assimp/assimp)** - Asset importing (OBJ/FBX/GLB/GLTF)
- **[filewatch](https://github.com/ThomasMonkman/filewatch)**
