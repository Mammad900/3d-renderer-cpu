#include "texture.h"
#include "object.h"
#include "data.h"

template <typename T>
inline T Texture<T>::sample(Fragment &f) {
    return sample(f.uv, f.dUVdx, f.dUVdy);
}

template class Texture<Color>;
template class Texture<float>;
template class Texture<Vec3>;

float SineWaveTexture::sample(Vector2f uv, Vector2f, Vector2f) {
    float x = orientation ? uv.y : uv.x;
    return a * sin(b*x + c*timing.totalTime + d) + e;
}

void SineWaveTexture::Gui(std::string label) {
    if(ImGui::TreeNode(label.c_str())) {
        ImGui::Text("asin(bx+c)+d");
        ImGui::DragFloat("a (output multiplier)", &a, 0.1f);
        ImGui::DragFloat("b (uv multiplier)", &b, 0.1f);
        ImGui::DragFloat("d (time multiplier)", &c, 0.1f);
        ImGui::DragFloat("d (uv constant)", &d, 0.1f);
        ImGui::DragFloat("e (output constant)", &e, 0.1f);
        ImGui::Checkbox("Orientation", &orientation);
        ImGui::TreePop();
    }
}
