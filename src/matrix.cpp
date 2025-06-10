#include <SFML/System/Vector3.hpp>
#include <array>
#include <math.h>
#include <vector>
#include "matrix.h"

using sf::Vector3f;

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

Vector3f operator* (const Vector3f &a, const TransformMatrix &b) {
    float arr[4] = {a.x, a.y, a.z, 1};
    matMul(arr, b.data(), arr, 1, 4, 4);
    return {arr[0], arr[1], arr[2]};
}

TransformMatrix makeRotationMatrix(Vector3f R) {
    Vector3f s = {sin(R.x), sin(R.y), sin(R.z)};
    Vector3f c = {cos(R.x), cos(R.y), cos(R.z)};

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
TransformMatrix makeTransformMatrix(TransformMatrix R, Vector3f S, Vector3f T) {
    return TransformMatrix{
        S.x, 0, 0, 0,
        0, S.y, 0, 0,
        0, 0, S.z, 0,
        0, 0, 0, 1,
    } * R * TransformMatrix{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        T.x, T.y, T.z, 1,
    };
}
TransformMatrix makeTransformMatrix(Vector3f R, Vector3f S, Vector3f T) {
    return makeTransformMatrix(makeRotationMatrix(R), S, T);
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
