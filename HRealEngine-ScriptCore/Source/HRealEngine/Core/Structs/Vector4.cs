
using System;
namespace HRealEngine
{
    public struct Vector4
    {
        public float X, Y, Z, W;

        public static Vector4 Zero => new Vector4(0.0f);

        public Vector4(float scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
            W = scalar;
        }

        public Vector4(float x, float y, float z, float w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        public Vector4(Vector3 xyz, float w)
        {
            X = xyz.X;
            Y = xyz.Y;
            Z = xyz.Z;
            W = w;
        }

        public Vector2 XY
        {
            get => new Vector2(X, Y);
            set
            {
                X = value.X;
                Y = value.Y;
            }
        }

        public Vector3 XYZ
        {
            get => new Vector3(X, Y, Z);
            set
            {
                X = value.X;
                Y = value.Y;
                Z = value.Z;
            }
        }

        public static Vector4 operator +(Vector4 a, Vector4 b)
        {
            return new Vector4(a.X + b.X, a.Y + b.Y, a.Z + b.Z, a.W + b.W);
        }
        public static Vector4 operator -(Vector4 a, Vector4 b)
        {
            return new Vector4(a.X - b.X, a.Y - b.Y, a.Z - b.Z, a.W - b.W);
        }
        public static Vector4 operator *(Vector4 vector, float scalar)
        {
            return new Vector4(vector.X * scalar, vector.Y * scalar, vector.Z * scalar, vector.W * scalar);
        }
        public static Vector4 operator /(Vector4 a, float scalar)
        {
            return new Vector4(a.X / scalar, a.Y / scalar, a.Z / scalar, a.W / scalar);
        }
        
        public override int GetHashCode() => (X, Y, Z, W).GetHashCode();
        public override bool Equals(object obj) => obj is Vector4 v && X == v.X && Y == v.Y && Z == v.Z && W == v.W;
        public static Vector4 operator ==(Vector4 a, Vector4 b)
        {
            return new Vector4(a.X == b.X ? 1 : 0, a.Y == b.Y ? 1 : 0, a.Z == b.Z ? 1 : 0, a.W == b.W ? 1 : 0);
        }
        public static Vector4 operator !=(Vector4 a, Vector4 b) 
        {
            return new Vector4(a.X != b.X ? 1 : 0, a.Y != b.Y ? 1 : 0, a.Z != b.Z ? 1 : 0, a.W != b.W ? 1 : 0);
        }

        public float Length()
        {
            return (float)Math.Sqrt(X * X + Y * Y + Z * Z + W * W);
        }

        public float LengthSquared()
        {
            return X * X + Y * Y + Z * Z + W * W;
        }

        public Vector4 Normalized()
        {
            float len = Length();
            return len > 0 ? this / len : Zero;
        }

        public void Normalize()
        {
            float len = Length();
            if (len > 0) { X /= len; Y /= len; Z /= len; W /= len; }
        }

        public static float Dot(Vector4 a, Vector4 b)
        {
            return a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;
        }

        public static float Distance(Vector4 a, Vector4 b)
        {
            return (a - b).Length();
        }

    }
}