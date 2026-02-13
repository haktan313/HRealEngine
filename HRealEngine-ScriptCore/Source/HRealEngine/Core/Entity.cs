
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
    }
}