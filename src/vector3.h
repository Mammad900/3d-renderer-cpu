#ifndef __VECTOR3_H__
#define __VECTOR3_H__

#include <SFML/System/Vector3.hpp>
#include <cmath>
#include <cassert>
#include <algorithm>

class Vec3 {
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// Creates a `Vec3(0, 0, 0)`.
    ///
    ////////////////////////////////////////////////////////////
    constexpr Vec3() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Construct the vector from its coordinates
    ///
    /// \param x X coordinate
    /// \param y Y coordinate
    /// \param z Z coordinate
    ///
    ////////////////////////////////////////////////////////////
    constexpr Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    ////////////////////////////////////////////////////////////
    /// \brief Converts the vector to another type of vector
    ///
    ////////////////////////////////////////////////////////////
    template <typename U>
    constexpr explicit operator sf::Vector3<U>() const {
        return {(U)x, (U)y, (U)z};
    }

    ////////////////////////////////////////////////////////////
    /// \brief Length of the vector <i><b>(floating-point)</b></i>.
    ///
    /// If you are not interested in the actual length, but only in comparisons, consider using `lengthSquared()`.
    ///
    ////////////////////////////////////////////////////////////
    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Square of vector's length.
    ///
    /// Suitable for comparisons, more efficient than `length()`.
    ///
    ////////////////////////////////////////////////////////////
    constexpr float lengthSquared() const {
        return dot(*this);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Vector with same direction but length 1 <i><b>(floating-point)</b></i>.
    ///
    /// \pre `*this` is no zero vector.
    ///
    ////////////////////////////////////////////////////////////
    Vec3 normalized() const;

    ////////////////////////////////////////////////////////////
    /// \brief Dot product of two 3D vectors.
    ///
    ////////////////////////////////////////////////////////////
    constexpr float dot(const Vec3& rhs) const { 
        return x * rhs.x + y * rhs.y + z * rhs.z;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Cross product of two 3D vectors.
    ///
    ////////////////////////////////////////////////////////////
    constexpr Vec3 cross(const Vec3& rhs) const { 
        return Vec3((y * rhs.z) - (z * rhs.y), (z * rhs.x) - (x * rhs.z), (x * rhs.y) - (y * rhs.x));
    }

    ////////////////////////////////////////////////////////////
    /// \brief Component-wise multiplication of `*this` and `rhs`.
    ///
    /// Computes `(lhs.x*rhs.x, lhs.y*rhs.y, lhs.z*rhs.z)`.
    ///
    /// Scaling is the most common use case for component-wise multiplication/division.
    /// This operation is also known as the Hadamard or Schur product.
    ///
    ////////////////////////////////////////////////////////////
    constexpr Vec3 componentWiseMul(const Vec3& rhs) const { 
        return Vec3(x * rhs.x, y * rhs.y, z * rhs.z); 
    }

    ////////////////////////////////////////////////////////////
    /// \brief Component-wise division of `*this` and `rhs`.
    ///
    /// Computes `(lhs.x/rhs.x, lhs.y/rhs.y, lhs.z/rhs.z)`.
    ///
    /// Scaling is the most common use case for component-wise multiplication/division.
    ///
    /// \pre Neither component of `rhs` is zero.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr Vec3 componentWiseDiv(const Vec3& rhs) const {
        assert(rhs.x != 0 && "Vec3::componentWiseDiv() cannot divide by 0");
        assert(rhs.y != 0 && "Vec3::componentWiseDiv() cannot divide by 0");
        assert(rhs.z != 0 && "Vec3::componentWiseDiv() cannot divide by 0");
        return Vec3(x / rhs.x, y / rhs.y, z / rhs.z);
    }

    constexpr Vec3 clamp(float min, float max) const {
        return {
            std::clamp(x, min, max),
            std::clamp(y, min, max),
            std::clamp(z, min, max),
        };
    }

    constexpr Vec3 pow(float a) const {
        return {
            std::pow(x, a),
            std::pow(y, a),
            std::pow(z, a),
        };
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    float x{}; //!< X coordinate of the vector
    float y{}; //!< Y coordinate of the vector
    float z{}; //!< Z coordinate of the vector
    // float w{}; // Fourth float to alow compiler auto-vectorization EDIT: doesn't actually speed up
};

////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of unary `operator-`
///
/// \param left Vector to negate
///
/// \return Member-wise opposite of the vector
///
////////////////////////////////////////////////////////////
[[nodiscard]] constexpr Vec3 operator-(const Vec3& left) {
    return Vec3(-left.x, -left.y, -left.z);
}

////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator+=`
///
/// This operator performs a member-wise addition of both vectors,
/// and assigns the result to `left`.
///
/// \param left  Left operand (a vector)
/// \param right Right operand (a vector)
///
/// \return Reference to `left`
///
////////////////////////////////////////////////////////////
constexpr Vec3& operator+=(Vec3& left, const Vec3& right) {
    left.x += right.x;
    left.y += right.y;
    left.z += right.z;
    return left;
}

////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator-=`
///
/// This operator performs a member-wise subtraction of both vectors,
/// and assigns the result to `left`.
///
/// \param left  Left operand (a vector)
/// \param right Right operand (a vector)
///
/// \return Reference to `left`
///
////////////////////////////////////////////////////////////
constexpr Vec3& operator-=(Vec3& left, const Vec3& right) {
    left.x -= right.x;
    left.y -= right.y;
    left.z -= right.z;
    return left;
}

////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator+`
///
/// \param left  Left operand (a vector)
/// \param right Right operand (a vector)
///
/// \return Member-wise addition of both vectors
///
////////////////////////////////////////////////////////////
[[nodiscard]] constexpr Vec3 operator+(const Vec3& left, const Vec3& right) {
    return Vec3(left.x + right.x, left.y + right.y, left.z + right.z);
}

////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator-`
///
/// \param left  Left operand (a vector)
/// \param right Right operand (a vector)
///
/// \return Member-wise subtraction of both vectors
///
////////////////////////////////////////////////////////////
[[nodiscard]] constexpr Vec3 operator-(const Vec3& left, const Vec3& right) {
    return Vec3(left.x - right.x, left.y - right.y, left.z - right.z);
}

////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator*`
///
/// \param left  Left operand (a vector)
/// \param right Right operand (a scalar value)
///
/// \return Member-wise multiplication by `right`
///
////////////////////////////////////////////////////////////
[[nodiscard]] constexpr Vec3 operator*(const Vec3& left, float right) {
    return Vec3(left.x * right, left.y * right, left.z * right);
}

////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator*`
///
/// \param left  Left operand (a scalar value)
/// \param right Right operand (a vector)
///
/// \return Member-wise multiplication by `left`
///
////////////////////////////////////////////////////////////
[[nodiscard]] constexpr Vec3 operator*(float left, const Vec3& right) {
    return Vec3(right.x * left, right.y * left, right.z * left);
}

////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator*=`
///
/// This operator performs a member-wise multiplication by `right`,
/// and assigns the result to `left`.
///
/// \param left  Left operand (a vector)
/// \param right Right operand (a scalar value)
///
/// \return Reference to `left`
///
////////////////////////////////////////////////////////////
constexpr Vec3& operator*=(Vec3& left, float right) {
    left.x *= right;
    left.y *= right;
    left.z *= right;
    return left;
}

////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator/`
///
/// \param left  Left operand (a vector)
/// \param right Right operand (a scalar value)
///
/// \return Member-wise division by `right`
///
////////////////////////////////////////////////////////////
[[nodiscard]] constexpr Vec3 operator/(const Vec3& left, float right) {
    assert(right != 0 && "Vec3::operator/ cannot divide by 0");
    return Vec3(left.x / right, left.y / right, left.z / right);
}

////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator/=`
///
/// This operator performs a member-wise division by `right`,
/// and assigns the result to `left`.
///
/// \param left  Left operand (a vector)
/// \param right Right operand (a scalar value)
///
/// \return Reference to `left`
///
////////////////////////////////////////////////////////////
constexpr Vec3& operator/=(Vec3& left, float right) {
    assert(right != 0 && "Vec3::operator/= cannot divide by 0");
    left.x /= right;
    left.y /= right;
    left.z /= right;
    return left;
}

////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator==`
///
/// This operator compares strict equality between two vectors.
///
/// \param left  Left operand (a vector)
/// \param right Right operand (a vector)
///
/// \return `true` if `left` is equal to `right`
///
////////////////////////////////////////////////////////////
[[nodiscard]] constexpr bool operator==(const Vec3& left, const Vec3& right) {
    return (left.x == right.x) && (left.y == right.y) && (left.z == right.z);
}

////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator!=`
///
/// This operator compares strict difference between two vectors.
///
/// \param left  Left operand (a vector)
/// \param right Right operand (a vector)
///
/// \return `true` if `left` is not equal to `right`
///
////////////////////////////////////////////////////////////
[[nodiscard]] constexpr bool operator!=(const Vec3& left, const Vec3& right) {
    return !(left == right);
}

#endif /* __VECTOR3_H__ */
