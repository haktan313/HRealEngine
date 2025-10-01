

namespace HRealEngine
{
    public class Input
    {
        public static bool IsKeyDown(KeyCodes keycode)
        {
            return InternalCalls.Input_IsKeyDown(keycode);
        }
    }
}