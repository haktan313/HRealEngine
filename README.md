# HRealEngine

**HRealEngine** is a custom **Game Engine** written in C++ built with **OpenGL**. It started as a **2D focused engine**, and has been expanding toward **3D rendering** and **3D physics**.

- Start with cloning the repo with `git clone --recursive https://github.com/haktan313/HRealEngine`

- If you cloned it without the `--recursive` flag, initialize submodules manually `git submodule update --init`

- If you make changes to the build files or need to regenerate Visual Studio project files, run `scripts/Win-GenProjects.bat` this will call **premake5** and create the required `.sln` files.

## 📸 Screenshots
| 2D | 3D |
|--------|-------------|
| <img width="2555" height="1386" alt="image" src="https://github.com/user-attachments/assets/821f02a4-56d2-4494-86b3-265eb5e2189d" /> | <img width="1913" height="1137" alt="image" src="https://github.com/user-attachments/assets/8a9c1386-550d-4038-b17b-8183d732b641" /> |

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
  - Overlap begin/end events (2D)
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
- **3D Rendering (recently added)**
  - Basic 3D mesh rendering pipeline (currently primitive meshes such as cubes)
  - `MeshComponent` for rendering 3D entities
  - Texture assignment support for 3D meshes

### Core
- Input handling and event system
- Logging with spdlog

## Notes / Roadmap
- OBJ mesh loading and asset pipeline improvements  
- Asset / resource management system  
- **My own Behavior Tree library implementation**
- **My own Navigation Mesh library implementation**

Script Example (Overlap Events)

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
- **[filewatch](https://github.com/ThomasMonkman/filewatch)**
