#ifndef __MATRIX_H__
#define __MATRIX_H__
#include <SFML/System/Vector3.hpp>
#include <array>
#include <math.h>
#include <vector>

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

void matMul(float *a, float *b, float *out, int aRows, int aCols, int bCols) {
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

void makeRotationMatrix(Vector3f R, float *out, bool clear= true) {
    Vector3f s = {sin(R.x), sin(R.y), sin(R.z)};
    Vector3f c = {cos(R.x), cos(R.y), cos(R.z)};

    //x
    std::array<float, 16> rotMatrix = {
        1, 0  , 0  , 0,
        0, c.x, s.x, 0,
        0,-s.x, c.x, 0,
        0, 0  , 0  , 1,
    };
    if(clear)
        std::copy(rotMatrix.begin(), rotMatrix.end(), out);
    else
        matMul(out, rotMatrix.data(), out, 4, 4, 4);

    //y
    rotMatrix = {
        c.y,0 ,-s.y, 0,
        0  ,1 , 0   ,0,
        s.y,0 , c.y ,0,
        0  ,0 ,0    ,1,
    };
    matMul(out, rotMatrix.data(), out, 4, 4, 4);
    
    //z
    rotMatrix = {
        c.z, s.z, 0, 0,
        -s.z, c.z, 0,0,
        0,0,1,0,
        0,0,0,1,
    };
    matMul(out, rotMatrix.data(), out, 4, 4, 4);
}
void makeTransformMatrix(Vector3f R, Vector3f S, Vector3f T, float *out) {
    std::array<float, 16> scaleMatrix = {
        S.x, 0, 0, 0,
        0, S.y, 0, 0,
        0, 0, S.z, 0,
        0, 0, 0, 1,
    };
    std::copy(scaleMatrix.begin(), scaleMatrix.end(), out);
    makeRotationMatrix(R, out, false);
    matMul(out, (float[]){
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        T.x, T.y, T.z, 1,
    }, out, 4,4,4);
}

sf::Vector3f rotate(sf::Vector3f a, const sf::Vector3f &th)
{
    float sinX = sin(th.x);
    float cosX = cos(th.x);
    float sinY = sin(th.y);
    float cosY = cos(th.y);
    float sinZ = sin(th.z);
    float cosZ = cos(th.z);

    // Rotate around X (pitch)
    float x1 = a.x;
    float y1 = cosX * a.y - sinX * a.z;
    float z1 = sinX * a.y + cosX * a.z;

    // Rotate around Y (yaw)
    float x2 = cosY * x1 + sinY * z1;
    float y2 = y1;
    float z2 = -sinY * x1 + cosY * z1;

    // Rotate around Z (roll)
    float x3 = cosZ * x2 - sinZ * y2;
    float y3 = sinZ * x2 + cosZ * y2;
    float z3 = z2;

    return {x3, y3, z3};
}

#endif /* __MATRIX_H__ */
