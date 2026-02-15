

namespace HRealEngine
{
    public class Input
    {
        public static bool IsKeyDown(KeyCodes keycode)
        {
            return InternalCalls.Input_IsKeyDown(keycode);
        }
        public static void GetMousePosition(out Vector2 result)
        {
            InternalCalls.Input_GetMousePosition(out result);
        }
        public static void SetCursorMode(MouseCurserMode mode)
        {
            InternalCalls.Input_SetCursorMode(mode);
        }

        public static MouseCurserMode GetCursorMode()
        {
            return InternalCalls.Input_GetCursorMode();
        }
        
        public static bool IsMousePressed(MouseButton button)
        {
            return InternalCalls.Input_IsMousePressed(button);
        }
    }
}