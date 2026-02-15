namespace HRealEngine
{
    public class GameModeData
    {
        public static void SetStringData(string dataName, string value)
        {
            InternalCalls.GameModeData_SetStringData(dataName, value);
        }
        public static string GetStringData(string dataName)
        {
            return InternalCalls.GameModeData_GetStringData(dataName);
        }
        
        public static void SetIntData(string dataName, int value)
        {
            InternalCalls.GameModeData_SetIntData(dataName, value);
        }
        public static int GetIntData(string dataName)
        {
            return InternalCalls.GameModeData_GetIntData(dataName);
        }
        
        public static void SetFloatData(string dataName, float value)
        {
            InternalCalls.GameModeData_SetFloatData(dataName, value);
        }
        public static float GetFloatData(string dataName)
        {
            return InternalCalls.GameModeData_GetFloatData(dataName);
        }
        
        public static void SetBoolData(string dataName, bool value)
        {
            InternalCalls.GameModeData_SetBoolData(dataName, value);
        }
        public static bool GetBoolData(string dataName)
        {
            return InternalCalls.GameModeData_GetBoolData(dataName);
        }
        
        public static void SetVector2Data(string dataName, Vector2 value)
        {
            InternalCalls.GameModeData_SetVector2Data(dataName, ref value);
        }
        public static Vector2 GetVector2Data(string dataName)
        {
            InternalCalls.GameModeData_GetVector2Data(dataName, out Vector2 result);
            return result;
        }
        
        public static void SetVector3Data(string dataName, Vector3 value)
        {
            InternalCalls.GameModeData_SetVector3Data(dataName, ref value);
        }
        public static Vector3 GetVector3Data(string dataName)
        {            
            InternalCalls.GameModeData_GetVector3Data(dataName, out Vector3 result);
            return result;
        }
        
        public static void SetVector4Data(string dataName, Vector4 value)
        {
            InternalCalls.GameModeData_SetVector4Data(dataName, ref value);
        }
        public static Vector4 GetVector4Data(string dataName)
        {
            InternalCalls.GameModeData_GetVector4Data(dataName, out Vector4 result);
            return result;
        }
        
        public static void SetEntityData(string dataName, ulong entityID)
        {
            InternalCalls.GameModeData_SetEntityData(dataName, entityID);
        }
        public static ulong GetEntityData(string dataName)
        {            
            return InternalCalls.GameModeData_GetEntityData(dataName);
        }
    }
}