
using System;
using System.Runtime.CompilerServices;

namespace HRealEngine.Calls
{
    public static class InternalCalls_GameModeData
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void GameModeData_SetStringData(string dataName, string value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static string GameModeData_GetStringData(string dataName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void GameModeData_SetIntData(string dataName, int value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static int GameModeData_GetIntData(string dataName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void GameModeData_SetFloatData(string dataName, float value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float GameModeData_GetFloatData(string dataName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void GameModeData_SetBoolData(string dataName, bool value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool GameModeData_GetBoolData(string dataName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void GameModeData_SetVector2Data(string dataName, ref Vector2 value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void GameModeData_GetVector2Data(string dataName, out Vector2 result);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void GameModeData_SetVector3Data(string dataName, ref Vector3 value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void GameModeData_GetVector3Data(string dataName, out Vector3 result);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void GameModeData_SetVector4Data(string dataName, ref Vector4 value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void GameModeData_GetVector4Data(string dataName, out Vector4 result);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void GameModeData_SetEntityData(string dataName, ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static ulong GameModeData_GetEntityData(string dataName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool GameModeData_HasData(string dataName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void GameModeData_RemoveData(string dataName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void GameModeData_ClearAllData();
    }
}