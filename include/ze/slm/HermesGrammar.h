#pragma once

#include <string>
#include <vector>

namespace slm {

struct SchemaProperty {
    std::string name;
    std::string type;                           // "string", "number", "integer", "boolean", "array"
    std::vector<std::string> enumValues;        // Optional enum constraint
    std::string arrayItemType;                  // For type="array": item element type
    std::string description;                    // Human-readable description for prompt context
};

struct JSONSchema {
    std::string title;
    std::vector<SchemaProperty> properties;
    std::vector<std::string> required;
};

class HermesGrammar {
public:
    // Compile a JSONSchema into a GBNF grammar string for llama.cpp constrained sampling
    static std::string compileToGBNF(const JSONSchema& schema);

    // Convert a JSONSchema to a JSON Schema object string for REST API structured output
    static std::string toJsonSchemaString(const JSONSchema& schema);
    
    // Preset schemas for ZombieEngine M13 SLM generators
    static JSONSchema getDialogueSchema();
    static JSONSchema getRadioBroadcastSchema();
    static JSONSchema getLoreNoteSchema();
    static JSONSchema getBuildAdviceSchema();
};

} // namespace slm
