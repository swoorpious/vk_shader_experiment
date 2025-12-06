// copyright 2025 swaroop.

#ifndef VK_SHADER_EXP_MATH_H
#define VK_SHADER_EXP_MATH_H


namespace MathCore {
    template <typename T>
    struct Vector2 {
        T x, y;

        T operator+(const Vector2<T>& other) const {
            return x + other.x, y + other.y;
        }

        T operator-(const Vector2<T>& other) const {
            return x - other.x, y - other.y;
        }

        T operator*(const Vector2<T>& other) const {
            return x * other.x + y * other.y;
        }

        T operator/(const Vector2<T>& other) const {
            return x / other.x + y / other.y;
        }

        void operator++() {
            ++x;
            ++y;
        }
        void operator--() {
            --x;
            --y;
        }
        void operator+=(const Vector2<T>& other) {
            x += other.x;
            y += other.y;
        }
        void operator-=(const Vector2<T>& other) {
            x -= other.x;
            y -= other.y;
        }

        Vector2(T _x, T _y) {
            x = static_cast<T>(_x);
            y = static_cast<T>(_y);
        }

        Vector2() {
            Vector2(
                static_cast<T>(0.0f),
                static_cast<T>(0.0f)
                );
        }
    };

    template <typename T>
    struct Vector3 {
        T x, y, z;

        T operator+(const Vector3<T>& other) const {
            return x + other.x, y + other.y, z + other.z;
        }

        T operator-(const Vector3<T>& other) const {
            return x - other.x, y - other.y, z - other.z;
        }

        T operator*(const Vector3<T>& other) const {
            return x * other.x + y * other.y, z * other.z;
        }

        T operator/(const Vector3<T>& other) const {
            return x / other.x + y / other.y, z / other.z;
        }

        void operator++() {
            ++x;
            ++y;
            ++z;
        }
        void operator--() {
            --x;
            --y;
            --z;
        }
        void operator+=(const Vector3<T>& other) {
            x += other.x;
            y += other.y;
            z += other.z;
        }
        void operator-=(const Vector3<T>& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
        }

        Vector3(T _x, T _y, T _z) {
            x = static_cast<T>(_x);
            y = static_cast<T>(_y);
            z = static_cast<T>(_z);
        }

        Vector3() {
            Vector3(
                static_cast<T>(0.0f),
                static_cast<T>(0.0f),
                static_cast<T>(0.0f)
                );
        }
    };

    template <typename T>
    struct Vector4 {
        T x, y, z, w;

        T operator+(const Vector4<T>& other) const {
            return x + other.x, y + other.y, z + other.z, w + other.w;
        }

        T operator-(const Vector4<T>& other) const {
            return x - other.x, y - other.y, z - other.z, w - other.w;
        }

        T operator*(const Vector4<T>& other) const {
            return x * other.x + y * other.y, z * other.z, w * other.w;
        }

        T operator/(const Vector4<T>& other) const {
            return x / other.x + y / other.y, z / other.z, w / other.w;
        }

        void operator++() {
            ++x;
            ++y;
            ++z;
            ++w;
        }
        void operator--() {
            --x;
            --y;
            --z;
            --w;
        }
        void operator+=(const Vector4<T>& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            w += other.w;
        }
        void operator-=(const Vector4<T>& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            w -= other.w;
        }

        Vector4(T _x, T _y, T _z, T _w) {
            x = static_cast<T>(_x);
            y = static_cast<T>(_y);
            z = static_cast<T>(_z);
            w = static_cast<T>(_w);
        }

        Vector4() {
            Vector4(
                static_cast<T>(0.0f),
                static_cast<T>(0.0f),
                static_cast<T>(0.0f),
                static_cast<T>(0.0f)
                );
        }
    };
}

namespace Math {
    using namespace MathCore;

    typedef Vector2<float> Vector2f;
    typedef Vector2<double> Vector2d;
    typedef Vector2<int> Vector2i;

    typedef Vector3<float> Vector3f;
    typedef Vector3<double> Vector3d;
    typedef Vector3<int> Vector3i;

    typedef Vector4<float> Vector4f;
    typedef Vector4<double> Vector4d;
    typedef Vector4<int> Vector4i;
    
}


#endif //VK_SHADER_EXP_MATH_H