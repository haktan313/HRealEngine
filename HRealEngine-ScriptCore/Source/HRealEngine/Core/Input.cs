

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
    }
}