# HRealEngine

**HRealEngine** is a custom **2D Game Engine** built with **C++** and **OpenGL**.

- Start with cloning the repo with `git clone --recursive https://github.com/haktan313/HRealEngine`

- If you cloned it without the `--recursive` flag, initialize submodules manually `git submodule update --init`

- If you make changes to the build files or need to regenerate Visual Studio project files, run `scripts/Win-GenProjects.bat` this will call **premake5** and create the required `.sln` files.

## 📸 Screenshots
| Editor | Sample Scene |
|--------|-------------|
| <img width="2555" height="1386" alt="image" src="https://github.com/user-attachments/assets/821f02a4-56d2-4494-86b3-265eb5e2189d" /> | ![Scene Screenshot](https://github.com/user-attachments/assets/11c43985-ec0e-4c48-8372-daf0f393670a) |

🧩 Some Features
- ImGui scene editor with viewport and gizmos
- Entity Component System by entt
- C# scripting support with Mono
- Dual scripting support — C++ (Native) and C# (Mono)
- Some events like: OnCreate, OnUpdate, OnDestroy, OnOverlapBegin, OnOverlapEnd, Destroy, etc.
- 2D batch renderer with textures and shaders
- Scene serialization using YAML
- Input handling and event system
- Layer and application framework
- Logging with spdlog
- OpenGL rendering backend
- Hot reloadable C# assemblies
- Framebuffer and render command abstraction
- Orthographic camera and controller
- File dialogs and content browser panel

Script Example (Overlap Events Box2d)

```cpp
                if (!m_CollisionBeginEvents.empty())
                {
                    for (const auto& collisionEvent : m_CollisionBeginEvents)
                    {
                        if (m_Registry.any_of<ScriptComponent>(collisionEvent.A))
                        {
                            Entity entityA = {collisionEvent.A, this};
                            ScriptEngine::OnCollisionBegin2D(entityA, Entity{collisionEvent.B, this});
                        }
                        if (m_Registry.any_of<ScriptComponent>(collisionEvent.B))
                        {
                            Entity entityB = {collisionEvent.B, this};
                            ScriptEngine::OnCollisionBegin2D(entityB, Entity{collisionEvent.A, this});
                        }

                        if (m_Registry.any_of<NativeScriptComponent>(collisionEvent.A))
                        {
                            Entity entityA = {collisionEvent.A, this};
                            auto& nsc = entityA.GetComponent<NativeScriptComponent>();
                            if (nsc.Instance)
                                nsc.Instance->OnCollisionBegin2D(Entity{collisionEvent.B, this});
                        }
                        if (m_Registry.any_of<NativeScriptComponent>(collisionEvent.B))
                        {
                            Entity entityB = {collisionEvent.B, this};
                            auto& nsc = entityB.GetComponent<NativeScriptComponent>();
                            if (nsc.Instance)
                                nsc.Instance->OnCollisionBegin2D(Entity{collisionEvent.A, this});
                        }
                    }
                    m_CollisionBeginEvents.clear();
                }
                if (!m_CollisionEndEvents.empty())
                {
                    for (const auto& collisionEvent : m_CollisionEndEvents)
                    {
                        if (m_Registry.any_of<ScriptComponent>(collisionEvent.A))
                        {
                            Entity entityA = {collisionEvent.A, this};
                            ScriptEngine::OnCollisionEnd2D(entityA, Entity{collisionEvent.B, this});
                        }
                        if (m_Registry.any_of<ScriptComponent>(collisionEvent.B))
                        {
                            Entity entityB = {collisionEvent.B, this};
                            ScriptEngine::OnCollisionEnd2D(entityB, Entity{collisionEvent.A, this});
                        }

                        if (m_Registry.any_of<NativeScriptComponent>(collisionEvent.A))
                        {
                            Entity entityA = {collisionEvent.A, this};
                            auto& nsc = entityA.GetComponent<NativeScriptComponent>();
                            if (nsc.Instance)
                                nsc.Instance->OnCollisionEnd2D(Entity{collisionEvent.B, this});
                        }
                        if (m_Registry.any_of<NativeScriptComponent>(collisionEvent.B))
                        {
                            Entity entityB = {collisionEvent.B, this};
                            auto& nsc = entityB.GetComponent<NativeScriptComponent>();
                            if (nsc.Instance)
                                nsc.Instance->OnCollisionEnd2D(Entity{collisionEvent.A, this});
                        }
                    }
                    m_CollisionEndEvents.clear();
                }
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
