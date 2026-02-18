

using System;
using System.Runtime.CompilerServices;

namespace HRealEngine
{
    public static class InternalCalls
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(ulong entityID, Type componentType);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static ulong Entity_FindEntityByName(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static ulong Entity_GetHoveredEntity();
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void OpenScene(string scenePath);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static object GetScriptInstance(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void DestroyEntity(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static ulong SpawnEntity(string name, string tag, ref Vector3 translation, ref Vector3 rotation, ref Vector3 scale);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Entity_AddComponent(ulong entityID, Type componentType);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(KeyCodes keyCode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetMousePosition(out Vector2 result);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Input_SetCursorMode(MouseCurserMode mode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static MouseCurserMode Input_GetCursorMode();
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsMousePressed(MouseButton button);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetTranslation(ulong entityID, out Vector3 result);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetTranslation(ulong entityID, ref Vector3 value);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetRotation(ulong entityID, out Vector3 result); 
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetRotation(ulong entityID, ref Vector3 value);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static string TextComponent_GetText(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void TextComponent_SetText(ulong entityID, string text);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void TextComponent_GetColor(ulong entityID, out Vector4 color);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void TextComponent_SetColor(ulong entityID, ref Vector4 color);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float TextComponent_GetKerning(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void TextComponent_SetKerning(ulong entityID, float kerning);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float TextComponent_GetLineSpacing(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void TextComponent_SetLineSpacing(ulong entityID, float lineSpacing);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulse(ulong entityID, ref Vector2 impulse, ref Vector2 point, bool wake);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(ulong entityID, ref Vector2 impulse, bool wake);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_ApplyLinearImpulseToCenter(ulong entityID, ref Vector3 impulse);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_GetLinearVelocity(ulong entityID, out Vector3 velocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_SetLinearVelocity(ulong entityID, ref Vector3 velocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_SetRotationDegrees(ulong entityID, ref Vector3 eulerDeg);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_GetRotationDegrees(ulong entityID, out Vector3 result);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_SetBodyType(ulong entityID, RigidBodyType bodyType);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody3DComponent_GetBodyType(ulong entityID, out RigidBodyType bodyType);

        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void MeshRendererComponent_SetMesh(ulong entityID, string meshPath);
        
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider3DComponent_SetSize(ulong entityID, ref Vector3 size);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static Vector3 BoxCollider3DComponent_GetSize(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider3DComponent_SetOffset(ulong entityID, ref Vector3 offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static Vector3 BoxCollider3DComponent_GetOffset(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider3DComponent_SetIsTrigger(ulong entityID, bool isTrigger);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool BoxCollider3DComponent_GetIsTrigger(ulong entityID);
        

        
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