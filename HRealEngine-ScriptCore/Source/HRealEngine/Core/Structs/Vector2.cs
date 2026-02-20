
using System;
namespace HRealEngine
{
    public struct Vector2
    {
        public float X;
        public float Y;

        public Vector2(float x, float y)
        {
            X = x;
            Y = y;
        }
        
        public Vector2(float scalar)
        {
            X = scalar;
            Y = scalar;
        }

        public static Vector2 Zero => new Vector2(0, 0);

        public static Vector2 operator +(Vector2 a, Vector2 b) => new Vector2(a.X + b.X, a.Y + b.Y);
        public static Vector2 operator -(Vector2 a, Vector2 b) => new Vector2(a.X - b.X, a.Y - b.Y);
        public static Vector2 operator *(Vector2 a, float scalar) => new Vector2(a.X * scalar, a.Y * scalar);
        public static Vector2 operator /(Vector2 a, float scalar) => new Vector2(a.X / scalar, a.Y / scalar);

        public float Length()
        {
            return (float)Math.Sqrt(X * X + Y * Y);   
        }

        public float LengthSquared()
        {
            return X * X + Y * Y;
        }

        public Vector2 Normalized()
        {
            float len = Length();
            return len > 0 ? this / len : Zero;
        }

        public void Normalize()
        {
            float len = Length();
            if (len > 0)
                X /= len; Y /= len;
        }

        public static float Dot(Vector2 a, Vector2 b)
        {
            return a.X * b.X + a.Y * b.Y;
        }

        public static float Distance(Vector2 a, Vector2 b)
        {
            return (a - b).Length();
        }

        public override string ToString() => $"({X}, {Y})";
    }
}