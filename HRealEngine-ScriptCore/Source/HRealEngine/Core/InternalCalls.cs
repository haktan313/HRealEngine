

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
        internal extern static void OpenScene(string scenePath);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static object GetScriptInstance(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void DestroyEntity(ulong entityID);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(KeyCodes keyCode);
        
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
    }
}