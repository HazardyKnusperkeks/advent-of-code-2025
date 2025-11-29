#ifndef COORDINATE3D_HPP
#define COORDINATE3D_HPP

#include <functional>
#include <limits>

template<typename T>
struct Coordinate3D {
    T X;
    T Y;
    T Z;

    constexpr bool operator==(const Coordinate3D&) const noexcept  = default;
    constexpr auto operator<=>(const Coordinate3D&) const noexcept = default;

    constexpr Coordinate3D operator-(void) const noexcept {
        return {-X, -Y, -Z};
    }

    constexpr Coordinate3D operator+(const Coordinate3D& that) const noexcept {
        return {X + that.X, Y + that.Y, Z + that.Z};
    }

    constexpr Coordinate3D operator-(const Coordinate3D& that) const noexcept {
        return {X - that.X, Y - that.Y, Z - that.Z};
    }

    template<typename U>
    constexpr Coordinate3D operator*(U t) const noexcept {
        return {t * X, t * Y, t * Z};
    }

    template<typename U>
    constexpr Coordinate3D operator/(U t) const noexcept {
        return {X / t, Y / t, Z / t};
    }

    template<typename U>
    friend constexpr Coordinate3D operator*(U t, const Coordinate3D& c) noexcept {
        return c * t;
    }
};

namespace std {
template<typename T>
struct hash<Coordinate3D<T>> {
    size_t operator()(const Coordinate3D<T>& c) const noexcept {
        constexpr auto bits = std::numeric_limits<T>::digits / 3;
        constexpr auto mask = ((T{1} << bits) - 1);
        return std::hash<T>{}((c.X << (2 * bits)) | ((c.Y & mask) << bits) | (c.Z & mask));
    }
};
} //namespace std

#endif //COORDINATE3D_HPP
