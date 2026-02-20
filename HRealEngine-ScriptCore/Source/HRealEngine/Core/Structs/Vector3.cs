
using System;
namespace HRealEngine
{
    public struct Vector3
    {
        public float X;
        public float Y;
        public float Z;
        
        public Vector3(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }
        public Vector3(float scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
        }
        
        public Vector3(Vector2 xy, float z)
        {
            X = xy.X;
            Y = xy.Y;
            Z = z;
        }

        public Vector2 XY
        {
            get => new Vector2(X, Y);
            set
            { X = value.X; Y = value.Y; }
        }
        
        public static Vector3 Zero => new Vector3(0.0f);
        
        public static Vector3 operator +(Vector3 a, Vector3 b) => new Vector3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
        public static Vector3 operator -(Vector3 a, Vector3 b) => new Vector3(a.X - b.X, a.Y - b.Y, a.Z - b.Z);
        public static Vector3 operator *(Vector3 a, float scalar) => new Vector3(a.X * scalar, a.Y * scalar, a.Z * scalar);
        public static Vector3 operator /(Vector3 a, float scalar) => new Vector3(a.X / scalar, a.Y / scalar, a.Z / scalar);
        
        public override int GetHashCode() => (X, Y, Z).GetHashCode();
        public override bool Equals(object obj) => obj is Vector3 v && X == v.X && Y == v.Y && Z == v.Z;
        public static Vector3 operator ==(Vector3 a, Vector3 b) => new Vector3(a.X == b.X ? 1 : 0, a.Y == b.Y ? 1 : 0, a.Z == b.Z ? 1 : 0);
        public static Vector3 operator !=(Vector3 a, Vector3 b) => new Vector3(a.X != b.X ? 1 : 0, a.Y != b.Y ? 1 : 0, a.Z != b.Z ? 1 : 0);

        public float Length()
        {
            return (float)Math.Sqrt(X * X + Y * Y + Z * Z);
        }

        public float LengthSquared()
        {
            return X * X + Y * Y + Z * Z;
        }

        public Vector3 Normalized()
        {
            float len = Length();
            return len > 0 ? this / len : Zero;
        }

        public void Normalize()
        {
            float len = Length();
            if (len > 0) { X /= len; Y /= len; Z /= len; }
        }

        public static float Dot(Vector3 a, Vector3 b)
        {
            return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
        }

        public static float Distance(Vector3 a, Vector3 b)
        {
            return (a - b).Length();
        }

        public static Vector3 Cross(Vector3 a, Vector3 b) => new Vector3(
            a.Y * b.Z - a.Z * b.Y,
            a.Z * b.X - a.X * b.Z,
            a.X * b.Y - a.Y * b.X
        );
    }
}