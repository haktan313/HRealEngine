
using System;
using HRealEngine.Calls;

namespace HRealEngine
{
    public class Entity
    {
        protected Entity() { EntityID = 0; }
        internal Entity(ulong entityID)        
        {
            EntityID = entityID;
        }
        public readonly ulong EntityID;
        
        public virtual void OnEntityPerceived(ulong entityID, int perceptionMethod, Vector3 position) {}
        public virtual void OnEntityLost(ulong entityID, Vector3 lastKnownPosition) {}
        public virtual void OnEntityForgotten(ulong entityID) {}
        
        public Entity Parent
        {
            get
            {
                ulong parentID = InternalCalls_Entity.Entity_GetParent(EntityID);
                return parentID == 0 ? null : new Entity(parentID);
            }
            set
            {
                if (value == null)
                    InternalCalls_Entity.Entity_RemoveParent(EntityID);
                else
                    InternalCalls_Entity.Entity_SetParent(EntityID, value.EntityID);
            }
        }
        public int ChildCount => InternalCalls_Entity.Entity_GetChildCount(EntityID);
        public void AddChild(Entity child)
        {
            if (child != null)
                InternalCalls_Entity.Entity_AddChild(EntityID, child.EntityID);
        }
        public Entity GetChild(int index)
        {
            ulong childID = InternalCalls_Entity.Entity_GetChild(EntityID, index);
            return childID == 0 ? null : new Entity(childID);
        }
        
        public Vector3 Position
        {
            get
            {
                InternalCalls_TransformComponent.TransformComponent_GetPosition(EntityID, out Vector3 result);
                return result;
            }
            set
            {
                InternalCalls_TransformComponent.TransformComponent_SetPosition(EntityID, ref value);
            }
        }
        public Vector3 Rotation
        {
            get
            {
                InternalCalls_TransformComponent.TransformComponent_GetRotation(EntityID, out Vector3 r); 
                return r;
            }
            set
            {
                InternalCalls_TransformComponent.TransformComponent_SetRotation(EntityID, ref value);
            }
        }
        public string Name
        {
            get
            {
                return InternalCalls_Entity.Entity_GetName(EntityID);
            }
            set
            {
                InternalCalls_Entity.Entity_SetName(EntityID, value);
            }
        }
        public bool HasTag(string tag)
        {
            return InternalCalls_Entity.Entity_HasTag(EntityID, tag);
        }
        public bool HasComponent<T>() where T : Component, new()
        {
            return InternalCalls_Entity.Entity_HasComponent(EntityID, typeof(T));
        }
        public void AddComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>())
                return;
            InternalCalls_Entity.Entity_AddComponent(EntityID, typeof(T));
        }
        public void AddRigidbody3DComponent(RigidBodyType bodyType, bool fixedRotation, float friction, float restitution, float convexRadius)
        {
            if (HasComponent<Rigidbody3DComponent>())
                return;
            InternalCalls_Entity.Entity_AddRigidbody3DComponent(EntityID, bodyType, fixedRotation, friction, restitution, convexRadius);
        }
        public void AddBoxCollider3DComponent(bool isTrigger, Vector3 size, Vector3 offset)
        {
            if (HasComponent<BoxCollider3DComponent>())
                return;
            InternalCalls_Entity.Entity_AddBoxCollider3DComponent(EntityID, isTrigger, ref size, ref offset);
        }
        public void AddMeshRendererComponent(string meshPath)
        {
            if (HasComponent<MeshRendererComponent>())
                return;
            InternalCalls_Entity.Entity_AddMeshRendererComponent(EntityID, meshPath);
        }
        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;
            T component = new T();
            component.entity = this;
            return component;
        }
        public Entity GetHoveredEntity()
        {
            ulong entityID = InternalCalls_Entity.Entity_GetHoveredEntity();
            if (entityID == 0)
                return null;
            return new Entity(entityID);
        }
        public T As<T>() where T : Entity, new()
        {
            object instance = InternalCalls_GlobalCalls.GetScriptInstance(EntityID);
            return instance as T;
        }
        

        public Entity FromID(ulong entityID)
        {
            return GlobalFunctions.FromID(entityID);
        }
        public float GetDeltaTime()
        {
            return GlobalFunctions.GetDeltaTime();
        }
        public void OpenScene(string scenePath)
        {
            GlobalFunctions.OpenScene(scenePath);
        }
        public void Destroy(ulong entityID)
        {
            GlobalFunctions.Destroy(entityID);
        }
        public Entity SpawnEntity(string name, string tag, Vector3 translation, Vector3 rotation, Vector3 scale)
        {
            return GlobalFunctions.SpawnEntity(name, tag, translation, rotation, scale);
        }
        public Entity FindEntityByName(string name)
        {
            return GlobalFunctions.FindEntityByName(name);
        }
        public bool Raycast3D(Vector3 origin, Vector3 direction, float maxDistance, out RaycastHit hit, ulong[] ignoreEntities = null, bool debugDraw = false, float debugDrawDuration = 0.0f)
        {
            return GlobalFunctions.Raycast3D(origin, direction, maxDistance, out hit, ignoreEntities, debugDraw, debugDrawDuration);
        }
        public RaycastHit[] Raycast3DAll(Vector3 origin, Vector3 direction, float maxDistance, ulong[] ignoreEntities = null, bool debugDraw = false, float debugDrawDuration = 0.0f)
        {
            return GlobalFunctions.Raycast3DAll(origin, direction, maxDistance, ignoreEntities, debugDraw, debugDrawDuration);
        }
    }
}