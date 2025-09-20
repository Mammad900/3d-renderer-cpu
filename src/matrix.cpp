#include <array>
#include <math.h>
#include <vector>
#include "matrix.h"



void matAdd(float *a, float *b, float *out, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int index = i * cols + j;
            out[index] = a[index] + b[index];  // Add corresponding elements
        }
    }
}
void matSub(float *a, float *b, float *out, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int index = i * cols + j;
            out[index] = a[index] - b[index];  // Subtract corresponding elements
        }
    }
}

void matScalarMul(float *a, float scalar, float *out, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int index = i * cols + j;
            out[index] = a[index] * scalar;  // Multiply each element by the scalar
        }
    }
}

void matScalarDiv(float *a, float scalar, float *out, int rows, int cols) {
    if (scalar == 0) {
        return;
    }
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int index = i * cols + j;
            out[index] = a[index] / scalar;  // Divide each element by the scalar
        }
    }
}

void matMul(const float *a, const float *b, float *out, int aRows, int aCols, int bCols) {
    std::vector<float> temp(aRows * bCols);  // Temporary buffer

    for (int i = 0; i < aRows; ++i) {
        for (int j = 0; j < bCols; ++j) {
            temp[i * bCols + j] = 0;
            for (int k = 0; k < aCols; ++k) {
                temp[i * bCols + j] += a[i * aCols + k] * b[k * bCols + j];
            }
        }
    }

    // Copy result back to out
    std::copy(temp.begin(), temp.end(), out);
}

void makeIdentityMatrix(float *out, int size) {
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            out[i + size * j] = i == j ? 1 : 0;
}

TransformMatrix operator* (const TransformMatrix &a, const TransformMatrix &b) {
    TransformMatrix res;
    matMul(a.data(), b.data(), res.data(), 4, 4, 4);
    return res;
}

Vec3 operator* (const Vec3 &a, const TransformMatrix &m) {
    return {
        a.x * m[0] + a.y * m[4] + a.z * m[8]  + m[12],
        a.x * m[1] + a.y * m[5] + a.z * m[9]  + m[13],
        a.x * m[2] + a.y * m[6] + a.z * m[10] + m[14]
    };
}

TransformMatrix makeRotationMatrix(Vec3 R) {
    Vec3 s = {sin(R.x), sin(R.y), sin(R.z)};
    Vec3 c = {cos(R.x), cos(R.y), cos(R.z)};

    //x
    return TransformMatrix{
        1, 0  , 0  , 0,
        0, c.x, s.x, 0,
        0,-s.x, c.x, 0,
        0, 0  , 0  , 1,
    } * TransformMatrix{
        c.y,0 ,-s.y, 0,
        0  ,1 , 0   ,0,
        s.y,0 , c.y ,0,
        0  ,0 ,0    ,1,
    } * TransformMatrix{
        c.z, s.z, 0, 0,
        -s.z, c.z, 0,0,
        0,0,1,0,
        0,0,0,1,
    };
}

TransformMatrix transposeMatrix(TransformMatrix &mat) {
    TransformMatrix transposed;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            transposed[i * 4 + j] = mat[j * 4 + i];
        }
    }
    return transposed;
}

bool inverseMatrix(TransformMatrix &mat, TransformMatrix &out) {
    float det = mat[0] * (mat[5] * mat[10] - mat[6] * mat[9]) -
                mat[1] * (mat[4] * mat[10] - mat[6] * mat[8]) +
                mat[2] * (mat[4] * mat[9] - mat[5] * mat[8]);

    if (det == 0)
        return false; // not invertible

    float invDet = 1.0f / det;

    out[0] = (mat[5] * mat[10] - mat[6] * mat[9]) * invDet;
    out[1] = (mat[2] * mat[9] - mat[1] * mat[10]) * invDet;
    out[2] = (mat[1] * mat[6] - mat[2] * mat[5]) * invDet;
    out[4] = (mat[6] * mat[8] - mat[4] * mat[10]) * invDet;
    out[5] = (mat[0] * mat[10] - mat[2] * mat[8]) * invDet;
    out[6] = (mat[2] * mat[4] - mat[0] * mat[6]) * invDet;
    out[8] = (mat[4] * mat[9] - mat[5] * mat[8]) * invDet;
    out[9] = (mat[1] * mat[8] - mat[0] * mat[9]) * invDet;
    out[10] = (mat[0] * mat[5] - mat[1] * mat[4]) * invDet;

    // Set the rest of the matrix to identity
    out[3] = out[7] = out[11] = out[12] = out[13] = out[14] = 0;
    out[15] = 1;

    return true;
}
