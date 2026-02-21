
using System;
using System.Runtime.CompilerServices;

namespace HRealEngine.Calls
{
    public static class InternalCalls_Input
    {
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
    }
}