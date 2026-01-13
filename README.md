# HRealEngine

**HRealEngine** is a custom **Game Engine** written in C++ built with **OpenGL**. It started as a **2D focused engine**, and has been expanding toward **3D rendering** and **3D physics**.

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
  - [Notes / Roadmap](#notes--roadmap)
  - [Script Example (Overlap Events)](#script-example-overlap-events)
  - [Third-Party Submodules](#-third-party-submodules)

### BehaviorTreeLibrary Integration
**BehaviorTreeLibrary** Integration still in proccess and it is visible in behaviorTree branch.
- As it is not yet complete, there are errors and some integration deficiencies. 

- There is a new window for the behavior tree editor inside the engine, and this window is dockable.
- There is a component for assigning behavior trees to entities. Behavior trees currently start running when runtime begins. 
- Behavior Trees can be recognized from the asset manager, where you can drag and drop them from the content browser into the component. 


https://github.com/user-attachments/assets/02698e7a-1b2d-4dbf-8792-ef2dad4175cf


https://github.com/user-attachments/assets/47cb6072-ca52-478f-9e62-6ded3dbc4675

## 📸 Screenshots
| 3D | OBJ Mesh |
|--------|-------------|
| <img width="1913" height="1137" alt="image" src="https://github.com/user-attachments/assets/8a9c1386-550d-4038-b17b-8183d732b641" /> | <img width="1894" height="1141" alt="Screenshot 2026-01-02 161346" src="https://github.com/user-attachments/assets/4f5b43d3-7c31-4e98-a4a6-6c5ec470f71d" /> |
| 2D | Asset Registry |
| <img width="2555" height="1386" alt="image" src="https://github.com/user-attachments/assets/821f02a4-56d2-4494-86b3-265eb5e2189d" /> | <img width="1324" height="937" alt="Screenshot 2026-01-02 161443" src="https://github.com/user-attachments/assets/61ad632f-7c6c-4ee9-8fea-8b7d2a2a99f6" /> |

### Editor & Workflow
- ImGui-based scene editor with viewport and gizmos
- Content Browser + file dialogs
- Scene serialization with YAML
- Entity Component System (entt)

### Scripting
- Dual scripting support:
  - C++ Native scripts
  - C# scripting with Mono (hot reloadable assemblies)
- Typical script events:
  - OnCreate, OnUpdate, OnDestroy, OnOverlapBegin, OnOverlapEnd, Destroy, etc.
  - Overlap begin/end events
  - Entity lifetime utilities (Destroy, etc.)

### Physics
- **2D Physics (Box2D)**
  - Rigidbody2D and collider components integrated into the ECS
  - Collision and overlap events forwarded to scripts
- **3D Physics (Jolt Physics)(recently added)**
  - Initial 3D physics pipeline
  - Rigidbody3D and basic 3D collider support

### Rendering
- OpenGL rendering
- Framebuffer + RenderCommand / Renderer abstractions
- Orthographic and Perspective cameras with controllers
- **2D Renderer**
  - Batch renderer with textures and shaders
- **3D Rendering**
  - `MeshComponent` for rendering 3D entities using mesh and material assets
  - `OBJ` mesh import and asset based rendering pipeline
  - `Material` system with texture support
  - Texture assignment support for 3D meshes

### Asset System
- Centralized asset registry and asset handles
- Import pipeline for meshes, materials, scenes, and textures
- Asset based references used across rendering and scene systems

### Project System
- Project configuration and asset directory management
- Per project asset registry and settings
- Support for opening and switching projects inside the editor

### Core
- Input handling and event system
- Logging with spdlog

## Notes / Roadmap
- **My own Behavior Tree library implementation**
- **My own Navigation Mesh library implementation**

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
- **[asimp](https://github.com/AminAliari/assimp.git)** - for OBJ
- **[filewatch](https://github.com/ThomasMonkman/filewatch)**
