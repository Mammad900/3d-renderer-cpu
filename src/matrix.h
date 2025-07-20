#ifndef __MATRIX_H__
#define __MATRIX_H__
#include <array>
#include <math.h>
#include <vector>
#include "vector3.h"


using TransformMatrix = std::array<float, 16>;
void matAdd(float *a, float *b, float *out, int rows, int cols);
void matSub(float *a, float *b, float *out, int rows, int cols);
void matScalarMul(float *a, float scalar, float *out, int rows, int cols);
void matScalarDiv(float *a, float scalar, float *out, int rows, int cols);
void matMul(const float *a, const float *b, float *out, int aRows, int aCols, int bCols);
void makeIdentityMatrix(float *out, int size);

TransformMatrix operator*(const TransformMatrix &a, const TransformMatrix &b);
Vec3 operator* (const Vec3 &a, const TransformMatrix &b);
TransformMatrix makeRotationMatrix(Vec3 R);
TransformMatrix transposeMatrix(TransformMatrix &mat);
bool inverseMatrix(TransformMatrix &mat, TransformMatrix &out);

#endif /* __MATRIX_H__ */
