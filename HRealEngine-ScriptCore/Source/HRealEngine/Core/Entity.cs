

using HRealEngine.HRealEngine.Core;

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
    }
}