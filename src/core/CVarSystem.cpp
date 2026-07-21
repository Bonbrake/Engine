#include "ze/core/CVarSystem.h"
#include <imgui.h>

namespace core {

CVarSystem& CVarSystem::Get() {
    static CVarSystem instance;
    return instance;
}

CVar* CVarSystem::RegisterInt(const char* name, int defaultValue) {
    auto& cvar = cvars[name];
    cvar.name = name;
    cvar.type = CVar::Type::INT;
    cvar.val.i = defaultValue;
    return &cvar;
}

CVar* CVarSystem::RegisterFloat(const char* name, float defaultValue) {
    auto& cvar = cvars[name];
    cvar.name = name;
    cvar.type = CVar::Type::FLOAT;
    cvar.val.f = defaultValue;
    return &cvar;
}

CVar* CVarSystem::RegisterBool(const char* name, bool defaultValue) {
    auto& cvar = cvars[name];
    cvar.name = name;
    cvar.type = CVar::Type::BOOL;
    cvar.val.b = defaultValue;
    return &cvar;
}

CVar* CVarSystem::GetCVar(const char* name) {
    auto it = cvars.find(name);
    if (it != cvars.end()) {
        return &it->second;
    }
    return nullptr;
}

int CVarSystem::GetInt(const char* name, int defaultValue) {
    auto* cvar = GetCVar(name);
    return cvar && cvar->type == CVar::Type::INT ? cvar->val.i : defaultValue;
}

float CVarSystem::GetFloat(const char* name, float defaultValue) {
    auto* cvar = GetCVar(name);
    return cvar && cvar->type == CVar::Type::FLOAT ? cvar->val.f : defaultValue;
}

bool CVarSystem::GetBool(const char* name, bool defaultValue) {
    auto* cvar = GetCVar(name);
    return cvar && cvar->type == CVar::Type::BOOL ? cvar->val.b : defaultValue;
}

void CVarSystem::DrawImGui() {
    ImGui::Text("CVars");
    ImGui::Separator();
    for (auto& [name, cvar] : cvars) {
        switch (cvar.type) {
            case CVar::Type::INT:
                ImGui::DragInt(cvar.name.c_str(), &cvar.val.i);
                break;
            case CVar::Type::FLOAT:
                ImGui::DragFloat(cvar.name.c_str(), &cvar.val.f);
                break;
            case CVar::Type::BOOL:
                ImGui::Checkbox(cvar.name.c_str(), &cvar.val.b);
                break;
        }
    }
}

} // namespace core