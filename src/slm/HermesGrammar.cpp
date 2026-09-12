#include "ze/slm/HermesGrammar.h"
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

namespace slm {

std::string HermesGrammar::compileToGBNF(const JSONSchema& schema) {
    std::ostringstream ss;
    
    // Root rule: ordered JSON object with all properties
    ss << "root ::= \"{\" ws ";
    for (size_t i = 0; i < schema.properties.size(); ++i) {
        const auto& prop = schema.properties[i];
        ss << "\"\\\"" << prop.name << "\\\":\" ws " << prop.name << "-val";
        if (i + 1 < schema.properties.size()) {
            ss << " \",\" ws ";
        }
    }
    ss << " ws \"}\"\n\n";

    // Value rules for each property
    bool needsStringRule = false;
    bool needsNumberRule = false;
    bool needsBoolRule = false;

    for (const auto& prop : schema.properties) {
        ss << prop.name << "-val ::= ";
        if (!prop.enumValues.empty()) {
            // Enum: constrain to exact quoted string values
            for (size_t j = 0; j < prop.enumValues.size(); ++j) {
                ss << "\"\\\"" << prop.enumValues[j] << "\\\"\"";
                if (j + 1 < prop.enumValues.size()) ss << " | ";
            }
        } else if (prop.type == "string") {
            ss << "string";
            needsStringRule = true;
        } else if (prop.type == "number" || prop.type == "integer") {
            ss << "number";
            needsNumberRule = true;
        } else if (prop.type == "boolean") {
            ss << "boolean";
            needsBoolRule = true;
        } else if (prop.type == "array") {
            // Typed array with configurable item type
            std::string itemRule = prop.arrayItemType.empty() ? "string" : prop.arrayItemType;
            ss << "\"[\" ws (" << itemRule << " (\",\" ws " << itemRule << ")*)? ws \"]\"";
            if (itemRule == "string") needsStringRule = true;
            if (itemRule == "number") needsNumberRule = true;
        } else {
            ss << "string";
            needsStringRule = true;
        }
        ss << "\n";
    }
    ss << "\n";

    // Standard primitive rules
    // String: supports JSON escape sequences including \uXXXX unicode escapes
    if (needsStringRule) {
        ss << "string ::= \"\\\"\" char* \"\\\"\"\n";
        ss << "char ::= [^\"\\\\\\x00-\\x1f] | \"\\\\\" escape\n";
        ss << "escape ::= [\"\\\\bfnrt/] | \"u\" [0-9a-fA-F] [0-9a-fA-F] [0-9a-fA-F] [0-9a-fA-F]\n";
    }

    // Number: supports negative, integer, and decimal
    if (needsNumberRule) {
        ss << "number ::= \"-\"? int frac?\n";
        ss << "int ::= \"0\" | [1-9] [0-9]*\n";
        ss << "frac ::= \".\" [0-9]+\n";
    }

    if (needsBoolRule) {
        ss << "boolean ::= \"true\" | \"false\"\n";
    }

    ss << "ws ::= [ \\t\\n\\r]*\n";

    return ss.str();
}

std::string HermesGrammar::toJsonSchemaString(const JSONSchema& schema) {
    json schemaObj;
    schemaObj["type"] = "object";

    json propsObj = json::object();
    json reqArr = json::array();

    for (const auto& prop : schema.properties) {
        json pObj;
        pObj["type"] = prop.type;
        if (!prop.description.empty()) {
            pObj["description"] = prop.description;
        }
        if (!prop.enumValues.empty()) {
            pObj["enum"] = prop.enumValues;
        }
        if (prop.type == "array" && !prop.arrayItemType.empty()) {
            json itemsObj;
            itemsObj["type"] = prop.arrayItemType;
            pObj["items"] = itemsObj;
        }
        propsObj[prop.name] = pObj;
    }

    for (const auto& r : schema.required) {
        reqArr.push_back(r);
    }

    schemaObj["properties"] = propsObj;
    schemaObj["required"] = reqArr;
    schemaObj["additionalProperties"] = false;

    json wrapper;
    wrapper["type"] = "json_schema";
    wrapper["json_schema"] = { {"name", schema.title}, {"strict", true}, {"schema", schemaObj} };

    return wrapper.dump(2);
}

JSONSchema HermesGrammar::getDialogueSchema() {
    JSONSchema s;
    s.title = "NPCDialogue";
    s.properties = {
        { "speaker", "string", {}, "", "Name of the speaking NPC" },
        { "emotion", "string", { "neutral", "fear", "anger", "grief", "hope", "surprise", "disgust" }, "", "Emotional state of the speaker" },
        { "text", "string", {}, "", "The dialogue line spoken by the NPC" }
    };
    s.required = { "speaker", "emotion", "text" };
    return s;
}

JSONSchema HermesGrammar::getRadioBroadcastSchema() {
    JSONSchema s;
    s.title = "RadioBroadcast";
    s.properties = {
        { "frequency", "number", {}, "", "Radio frequency in MHz (e.g. 104.5)" },
        { "callsign", "string", {}, "", "Station callsign identifier" },
        { "transmission", "string", {}, "", "The broadcast message content" }
    };
    s.required = { "frequency", "callsign", "transmission" };
    return s;
}

JSONSchema HermesGrammar::getLoreNoteSchema() {
    JSONSchema s;
    s.title = "EnvironmentalLore";
    s.properties = {
        { "author", "string", {}, "", "Author of the lore document" },
        { "category", "string", { "journal", "graffiti", "official_memo", "distress_letter", "research_log" }, "", "Type of lore artifact" },
        { "content", "string", {}, "", "The lore text content" }
    };
    s.required = { "author", "category", "content" };
    return s;
}

JSONSchema HermesGrammar::getBuildAdviceSchema() {
    JSONSchema s;
    s.title = "BuildAdvisor";
    s.properties = {
        { "playstyle", "string", {}, "", "Recommended playstyle archetype" },
        { "recommendedPerk", "string", {}, "", "Name of the recommended perk" },
        { "rationale", "string", {}, "", "Explanation of why this perk suits the playstyle" }
    };
    s.required = { "playstyle", "recommendedPerk", "rationale" };
    return s;
}

} // namespace slm
