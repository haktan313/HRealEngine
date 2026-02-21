

using HRealEngine.Calls;

namespace HRealEngine
{
    public class Input
    {
        public static bool IsKeyDown(KeyCodes keycode)
        {
            return InternalCalls_Input.Input_IsKeyDown(keycode);
        }
        public static void GetMousePosition(out Vector2 result)
        {
            InternalCalls_Input.Input_GetMousePosition(out result);
        }
        public static void SetCursorMode(MouseCurserMode mode)
        {
            InternalCalls_Input.Input_SetCursorMode(mode);
        }

        public static MouseCurserMode GetCursorMode()
        {
            return InternalCalls_Input.Input_GetCursorMode();
        }
        
        public static bool IsMousePressed(MouseButton button)
        {
            return InternalCalls_Input.Input_IsMousePressed(button);
        }
    }
}