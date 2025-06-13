#include "material.h"
#include <imgui.h>

void CheckboxFlag(const char *label, MaterialFlags &flags, MaterialFlags flag) {
    bool on = flags & flag;
    if(ImGui::Checkbox(label, &on))
        flags ^= flag;
}

void Material::GUI() {
    CheckboxFlag("Transparent", flags, MaterialFlags::Transparent);
    CheckboxFlag("DoubleSided", flags, MaterialFlags::DoubleSided);
    CheckboxFlag("AlphaCutout", flags, MaterialFlags::AlphaCutout);
}