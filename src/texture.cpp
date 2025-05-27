#include "texture.h"
#include "object.h"

template <typename T>
inline T Texture<T>::sample(Fragment &f) {
    return sample(f.uv, f.dUVdx, f.dUVdy);
}

template class Texture<Color>;
template class Texture<float>;
template class Texture<Vector3f>;
