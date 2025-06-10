#ifndef __MATRIX_H__
#define __MATRIX_H__
#include <SFML/System/Vector3.hpp>
#include <array>
#include <math.h>
#include <vector>

using sf::Vector3f;

using TransformMatrix = std::array<float, 16>;
void matAdd(float *a, float *b, float *out, int rows, int cols);
void matSub(float *a, float *b, float *out, int rows, int cols);
void matScalarMul(float *a, float scalar, float *out, int rows, int cols);
void matScalarDiv(float *a, float scalar, float *out, int rows, int cols);
void matMul(const float *a, const float *b, float *out, int aRows, int aCols, int bCols);
void makeIdentityMatrix(float *out, int size);

TransformMatrix operator*(const TransformMatrix &a, const TransformMatrix &b);
Vector3f operator* (const Vector3f &a, const TransformMatrix &b);
TransformMatrix makeRotationMatrix(Vector3f R);
TransformMatrix makeTransformMatrix(TransformMatrix R, Vector3f S, Vector3f T);
TransformMatrix makeTransformMatrix(Vector3f R, Vector3f S, Vector3f T);
TransformMatrix transposeMatrix(TransformMatrix &mat);

#endif /* __MATRIX_H__ */
