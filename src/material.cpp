#include "material.h"
#include <imgui.h>

bool CheckboxNP(const char *label, bool value) {
    return ImGui::Checkbox(label, &value);
}

void Material::GUI() {
    if(CheckboxNP("Transparent", flags.transparent))
        flags.transparent ^= true;
    if(CheckboxNP("DoubleSided", flags.doubleSided))
        flags.doubleSided ^= true;
    if(CheckboxNP("AlphaCutout", flags.alphaCutout))
        flags.alphaCutout ^= true;
}