#pragma once
#include <string>
#include <unordered_map>

namespace core {

struct CVar {
    enum class Type { INT, FLOAT, BOOL };
    std::string name;
    Type type;
    union {
        int i;
        float f;
        bool b;
    } val;
};

class CVarSystem {
public:
    static CVarSystem& Get();
    
    CVar* RegisterInt(const char* name, int defaultValue);
    CVar* RegisterFloat(const char* name, float defaultValue);
    CVar* RegisterBool(const char* name, bool defaultValue);
    
    CVar* GetCVar(const char* name);
    
    int GetInt(const char* name, int defaultValue = 0);
    float GetFloat(const char* name, float defaultValue = 0.0f);
    bool GetBool(const char* name, bool defaultValue = false);
    
    void DrawImGui();
private:
    CVarSystem() = default;
    std::unordered_map<std::string, CVar> cvars;
};

} // namespace core
