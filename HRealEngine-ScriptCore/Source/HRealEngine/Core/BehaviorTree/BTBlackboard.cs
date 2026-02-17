using System.Collections.Generic;

namespace HRealEngine.BehaviorTree
{
    public class BTBlackboard
    {
        private Dictionary<string, bool> boolValues = new Dictionary<string, bool>();
        private Dictionary<string, int> intValues = new Dictionary<string, int>();
        private Dictionary<string, float> floatValues = new Dictionary<string, float>();
        private Dictionary<string, string> stringValues = new Dictionary<string, string>();

        public bool GetBool(string key) { return boolValues.ContainsKey(key) ? boolValues[key] : false; } 
        public int GetInt(string key) { return intValues.ContainsKey(key) ? intValues[key] : 0; }
        public float GetFloat(string key) { return floatValues.ContainsKey(key) ? floatValues[key] : 0.0f; }
        public string GetString(string key) { return stringValues.ContainsKey(key) ? stringValues[key] : ""; }

        public void SetBool(string key, bool value) { boolValues[key] = value; }
        public void SetInt(string key, int value) { intValues[key] = value; }
        public void SetFloat(string key, float value) { floatValues[key] = value; }
        public void SetString(string key, string value) { stringValues[key] = value; }

        public bool HasBool(string key) { return boolValues.ContainsKey(key); }
        public bool HasInt(string key) { return intValues.ContainsKey(key); }
        public bool HasFloat(string key) { return floatValues.ContainsKey(key); }
        public bool HasString(string key) { return stringValues.ContainsKey(key); }

        protected void CreateBool(string key, bool value) { boolValues[key] = value; }
        protected void CreateInt(string key, int value) { intValues[key] = value; }
        protected void CreateFloat(string key, float value) { floatValues[key] = value; }
        protected void CreateString(string key, string value) { stringValues[key] = value; }
        
        
        public string[] GetBoolKeys()
        {
            var keys = new string[boolValues.Count];
            boolValues.Keys.CopyTo(keys, 0);
            return keys;
        }
        public bool[] GetBoolVals()
        {
            var vals = new bool[boolValues.Count];
            boolValues.Values.CopyTo(vals, 0);
            return vals;
        }
        public string[] GetIntKeys()
        {
            var keys = new string[intValues.Count];
            intValues.Keys.CopyTo(keys, 0);
            return keys;
        }
        public int[] GetIntVals()
        {
            var vals = new int[intValues.Count];
            intValues.Values.CopyTo(vals, 0);
            return vals;
        }
        public string[] GetFloatKeys()
        {
            var keys = new string[floatValues.Count];
            floatValues.Keys.CopyTo(keys, 0);
            return keys;
        }
        public float[] GetFloatVals()
        {
            var vals = new float[floatValues.Count];
            floatValues.Values.CopyTo(vals, 0);
            return vals;
        }
        public string[] GetStringKeys()
        {
            var keys = new string[stringValues.Count];
            stringValues.Keys.CopyTo(keys, 0);
            return keys;
        }
        public string[] GetStringVals()
        {
            var vals = new string[stringValues.Count];
            stringValues.Values.CopyTo(vals, 0);
            return vals;
        }
    }
}