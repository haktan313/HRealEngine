
using System;
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

        public Vector3 Translation
        {
            get
            {
                InternalCalls.TransformComponent_GetTranslation(EntityID, out Vector3 result);
                return result;
            }
            set
            {
                InternalCalls.TransformComponent_SetTranslation(EntityID, ref value);
            }
        }
        public Vector3 Rotation
        {
            get
            {
                InternalCalls.TransformComponent_GetRotation(EntityID, out Vector3 r); 
                return r;
            }
            set
            {
                InternalCalls.TransformComponent_SetRotation(EntityID, ref value);
            }
        }
        
        public bool HasComponent<T>() where T : Component, new()
        {
            return InternalCalls.Entity_HasComponent(EntityID, typeof(T));
        }
        public void AddComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>())
                return;
            InternalCalls.Entity_AddComponent(EntityID, typeof(T));
        }
        public void AddRigidbody3DComponent(RigidBodyType bodyType, bool fixedRotation, float friction, float restitution, float convexRadius)
        {
            if (HasComponent<Rigidbody3DComponent>())
                return;
            InternalCalls.Entity_AddRigidbody3DComponent(EntityID, bodyType, fixedRotation, friction, restitution, convexRadius);
        }
        public void AddBoxCollider3DComponent(bool isTrigger, Vector3 size, Vector3 offset)
        {
            if (HasComponent<BoxCollider3DComponent>())
                return;
            InternalCalls.Entity_AddBoxCollider3DComponent(EntityID, isTrigger, ref size, ref offset);
        }
        public void AddMeshRendererComponent(string meshPath)
        {
            if (HasComponent<MeshRendererComponent>())
                return;
            InternalCalls.Entity_AddMeshRendererComponent(EntityID, meshPath);
        }
        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;
            T component = new T();
            component.entity = this;
            return component;
        }

        public Entity FindEntityByName(string name)
        {
            ulong entityID = InternalCalls.Entity_FindEntityByName(name);
            if (entityID == 0)
                return null;
            return new Entity(entityID);
        }
        
        public Entity GetHoveredEntity()
        {
            ulong entityID = InternalCalls.Entity_GetHoveredEntity();
            if (entityID == 0)
                return null;
            return new Entity(entityID);
        }
        public T As<T>() where T : Entity, new()
        {
            object instance = InternalCalls.GetScriptInstance(EntityID);
            return instance as T;
        }
        
        public void Destroy(ulong entityID)
        {
            InternalCalls.DestroyEntity(entityID);
        }
        
        public void OpenScene(string scenePath)
        {
            InternalCalls.OpenScene(scenePath);
        }
        public Entity SpawnEntity(string name, string tag, Vector3 translation, Vector3 rotation, Vector3 scale)
        {
            ulong entityID = InternalCalls.SpawnEntity(name, tag, ref translation, ref rotation, ref scale);
            if (entityID == 0)
                return null;
            return new Entity(entityID);
        }
    }
}