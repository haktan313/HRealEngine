using System;
using System.Collections.Generic;
using System.Reflection;
using System.Linq;

namespace HRealEngine.BehaviorTree
{
    [AttributeUsage(AttributeTargets.Field)]
    public class BTParameterAttribute : Attribute
    {
        public string DisplayName;
        
        public BTParameterAttribute(string displayName = "")
        {
            DisplayName = displayName;
        }
    }

    [AttributeUsage(AttributeTargets.Field)]
    public class BTBlackboardKeyAttribute : Attribute
    {
        public enum KeyType
        {
            Float = 0,
            Int = 1,
            Bool = 2,
            String = 3
        }

        public KeyType Type;
        public string DisplayName;

        public BTBlackboardKeyAttribute(KeyType type, string displayName = "")
        {
            Type = type;
            DisplayName = displayName;
        }
    }
    
    public abstract class BTParams
    {
        public virtual void DrawImGui(BTBlackboard blackboard) { }

        public virtual Dictionary<string, object> Serialize()
        {
            var dict = new Dictionary<string, object>();
            var type = GetType();
            var fields = type.GetFields();

            foreach (var field in fields)
            {
                var paramAttr = (BTParameterAttribute)Attribute.GetCustomAttribute(field, typeof(BTParameterAttribute));
                var bbKeyAttr = (BTBlackboardKeyAttribute)Attribute.GetCustomAttribute(field, typeof(BTBlackboardKeyAttribute));

                if (paramAttr != null || bbKeyAttr != null)
                    dict[field.Name] = field.GetValue(this);
            }

            return dict;
        }
        public virtual void Deserialize(Dictionary<string, object> data)
        {
            var type = GetType();
            foreach (var keyValue in data)
            {
                var field = type.GetField(keyValue.Key);
                if (field != null)
                    field.SetValue(this, keyValue.Value);
            }
        }
        public virtual ParameterFieldInfo[] GetFieldInfos()
        {
            Type type = GetType();
            FieldInfo[] fields = type.GetFields(BindingFlags.Public | BindingFlags.Instance);
            List<ParameterFieldInfo> infos = new List<ParameterFieldInfo>();

            foreach (FieldInfo field in fields)
            {
                object[] paramAttrs = field.GetCustomAttributes(typeof(BTParameterAttribute), false);
                object[] bbKeyAttrs = field.GetCustomAttributes(typeof(BTBlackboardKeyAttribute), false);

                BTParameterAttribute paramAttr = paramAttrs.Length > 0 ? (BTParameterAttribute)paramAttrs[0] : null;
                BTBlackboardKeyAttribute bbKeyAttr = bbKeyAttrs.Length > 0 ? (BTBlackboardKeyAttribute)bbKeyAttrs[0] : null;

                if (paramAttr != null || bbKeyAttr != null)
                {
                    ParameterFieldInfo fieldInfo = new ParameterFieldInfo();
                    fieldInfo.Name = field.Name;
                    
                    if (paramAttr != null && !string.IsNullOrEmpty(paramAttr.DisplayName))
                        fieldInfo.DisplayName = paramAttr.DisplayName;
                    else if (bbKeyAttr != null && !string.IsNullOrEmpty(bbKeyAttr.DisplayName))
                        fieldInfo.DisplayName = bbKeyAttr.DisplayName;
                    else
                        fieldInfo.DisplayName = field.Name;
                    
                    fieldInfo.IsBlackboardKey = bbKeyAttr != null;
                    fieldInfo.BlackboardKeyType = bbKeyAttr != null ? (int)bbKeyAttr.Type : 0;
                    
                    infos.Add(fieldInfo);
                }
            }
            return infos.ToArray();
        }
    }
    
    public class ParameterFieldInfo
    {
        public string Name;
        public string DisplayName;
        public bool IsBlackboardKey;
        public int BlackboardKeyType; // 0=Float, 1=Int, 2=Bool, 3=String
    }
    
    public abstract class BTActionParams : BTParams { }
    public abstract class BTConditionParams : BTParams { }
    public abstract class BTDecoratorParams : BTParams { }
}