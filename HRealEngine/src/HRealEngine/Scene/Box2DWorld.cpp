#include "HRpch.h"
#include "Box2DWorld.h"

#include "Scene.h"
#include "ScriptableEntity.h"
#include "HRealEngine/Core/Components.h"
#include "HRealEngine/Scripting/ScriptEngine.h"

#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_contact.h"
#include "box2d/b2_circle_shape.h"
#include "box2d/b2_polygon_shape.h"

namespace HRealEngine
{
    static b2BodyType RigidBody2DTypeToBox2D(Rigidbody2DComponent::BodyType type)
    {
        switch (type)
        {
        case Rigidbody2DComponent::BodyType::Static:   return b2_staticBody;
        case Rigidbody2DComponent::BodyType::Dynamic:  return b2_dynamicBody;
        case Rigidbody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
        }
        HREALENGINE_CORE_DEBUGBREAK(false, "Unknown body type");
        return b2_staticBody;
    }

    Box2DWorld::Box2DWorld(Scene* scene) : m_Scene(scene)
    {
    }

    Box2DWorld::~Box2DWorld()
    {
        Stop2DPhysics();
    }

    void Box2DWorld::Init()
    {
        m_PhysicsWorld2D = new b2World({0.0f, -9.81f});
        m_PhysicsWorld2D->SetContactListener(new GameContactListener(m_Scene, this));

        auto view = m_Scene->GetRegistry().view<Rigidbody2DComponent>();
        for (auto e : view)
        {
            Entity entity = {e, m_Scene};
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

            b2BodyDef bodyDef;
            bodyDef.type = RigidBody2DTypeToBox2D(rb2d.Type);
            bodyDef.position.Set(transform.Position.x, transform.Position.y);
            bodyDef.angle = transform.Rotation.z;

            b2Body* body = m_PhysicsWorld2D->CreateBody(&bodyDef);
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

    void Box2DWorld::UpdateSimulation2D(Timestep deltaTime, int& stepFrames)
    {
        if (!m_Scene->IsPaused() || stepFrames-- > 0)
        {
            const int32_t velocityIterations = 6;
            const int32_t positionIterations = 2;
            m_PhysicsWorld2D->Step(deltaTime, velocityIterations, positionIterations);

            auto view = m_Scene->GetRegistry().view<Rigidbody2DComponent>();
            for (auto e : view)
            {
                Entity entity = {e, m_Scene};
                auto& transform = entity.GetComponent<TransformComponent>();
                auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

                b2Body* body = (b2Body*)rb2d.RuntimeBody;
                const auto& position = body->GetPosition();
                transform.Position.x = position.x;
                transform.Position.y = position.y;
                transform.Rotation.z = body->GetAngle();
            }
        }
    }

    void Box2DWorld::UpdateRuntime2D()
    {
        if (!m_CollisionBeginEvents.empty())
        {
            for (const auto& collisionEvent : m_CollisionBeginEvents)
            {
                if (m_Scene->GetRegistry().any_of<ScriptComponent>(collisionEvent.A))
                {
                    Entity entityA = {collisionEvent.A, m_Scene};
                    ScriptEngine::OnCollisionBegin2D(entityA, Entity{collisionEvent.B, m_Scene});
                }
                if (m_Scene->GetRegistry().any_of<ScriptComponent>(collisionEvent.B))
                {
                    Entity entityB = {collisionEvent.B, m_Scene};
                    ScriptEngine::OnCollisionBegin2D(entityB, Entity{collisionEvent.A, m_Scene});
                }       
                if (m_Scene->GetRegistry().any_of<NativeScriptComponent>(collisionEvent.A))
                {
                    Entity entityA = {collisionEvent.A, m_Scene};
                    auto& nsc = entityA.GetComponent<NativeScriptComponent>();
                    if (nsc.Instance)
                        nsc.Instance->OnCollisionBegin2D(Entity{collisionEvent.B, m_Scene});
                }
                if (m_Scene->GetRegistry().any_of<NativeScriptComponent>(collisionEvent.B))
                {
                    Entity entityB = {collisionEvent.B, m_Scene};
                    auto& nsc = entityB.GetComponent<NativeScriptComponent>();
                    if (nsc.Instance)
                        nsc.Instance->OnCollisionBegin2D(Entity{collisionEvent.A, m_Scene});
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
                    ScriptEngine::OnCollisionEnd2D(entityA, Entity{collisionEvent.B, m_Scene});
                }
                if (m_Scene->GetRegistry().any_of<ScriptComponent>(collisionEvent.B))
                {
                    Entity entityB = {collisionEvent.B, m_Scene};
                    ScriptEngine::OnCollisionEnd2D(entityB, Entity{collisionEvent.A, m_Scene});
                }       
                if (m_Scene->GetRegistry().any_of<NativeScriptComponent>(collisionEvent.A))
                {
                    Entity entityA = {collisionEvent.A, m_Scene};
                    auto& nsc = entityA.GetComponent<NativeScriptComponent>();
                    if (nsc.Instance)
                        nsc.Instance->OnCollisionEnd2D(Entity{collisionEvent.B, m_Scene});
                }
                if (m_Scene->GetRegistry().any_of<NativeScriptComponent>(collisionEvent.B))
                {
                    Entity entityB = {collisionEvent.B, m_Scene};
                    auto& nsc = entityB.GetComponent<NativeScriptComponent>();
                    if (nsc.Instance)
                        nsc.Instance->OnCollisionEnd2D(Entity{collisionEvent.A, m_Scene});
                }
            }
            m_CollisionEndEvents.clear();
        }
    }

    void Box2DWorld::Step2DWorld(Timestep deltaTime)
    {
        const int32_t velocityIterations = 6;
        const int32_t positionIterations = 2;
        m_PhysicsWorld2D->Step(deltaTime, velocityIterations, positionIterations);

        auto view = m_Scene->GetRegistry().view<Rigidbody2DComponent>();
        for (auto e : view)
        {
            Entity entity = {e, m_Scene};
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

            b2Body* body = (b2Body*)rb2d.RuntimeBody;
            const auto& position = body->GetPosition();
            transform.Position.x = position.x;
            transform.Position.y = position.y;
            transform.Rotation.z = body->GetAngle();
        }
    }

    void Box2DWorld::DestroyEntityPhysics(Entity entity)
    {
        if (entity.HasComponent<Rigidbody2DComponent>())
        {
            auto& rb = entity.GetComponent<Rigidbody2DComponent>();
            if (m_PhysicsWorld2D && rb.RuntimeBody)
            {
                b2Body* body = (b2Body*)rb.RuntimeBody;
                body->GetUserData().pointer = 0;
                m_PhysicsWorld2D->DestroyBody((b2Body*)rb.RuntimeBody);
                rb.RuntimeBody = nullptr;
            }
        }
    }

    void Box2DWorld::Stop2DPhysics()
    {
        delete m_PhysicsWorld2D;
        m_PhysicsWorld2D = nullptr;
    }

    void Box2DWorld::GameContactListener::BeginContact(b2Contact* contact)
    {
        b2Body* bodyA = contact->GetFixtureA()->GetBody();
        b2Body* bodyB = contact->GetFixtureB()->GetBody();

        auto entityA = m_Scene->GetEntityByUUID((UUID)bodyA->GetUserData().pointer);
        auto entityB = m_Scene->GetEntityByUUID((UUID)bodyB->GetUserData().pointer);
        if (m_Scene->GetRegistry().valid(entityA) && m_Scene->GetRegistry().valid(entityB))
            m_Box2DWorld->m_CollisionBeginEvents.push_back({ entityA, entityB });
    }

    void Box2DWorld::GameContactListener::EndContact(b2Contact* contact)
    {
        b2Body* bodyA = contact->GetFixtureA()->GetBody();
        b2Body* bodyB = contact->GetFixtureB()->GetBody();

        auto entityA = m_Scene->GetEntityByUUID((UUID)bodyA->GetUserData().pointer);
        auto entityB = m_Scene->GetEntityByUUID((UUID)bodyB->GetUserData().pointer);
        if (m_Scene->GetRegistry().valid(entityA) && m_Scene->GetRegistry().valid(entityB))
            m_Box2DWorld->m_CollisionEndEvents.push_back({ entityA, entityB });
    }
}
