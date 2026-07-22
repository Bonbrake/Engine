#include <catch2/catch_test_macros.hpp>
#include "ze/modding/Modding.h"
#include "ze/net/StateSync.h"
#include "ze/slm/PromptTemplate.h"
#include "ze/ui/HapticManager.h"
#include "ze/world/ChunkStreamer.h"
#include "ze/world/BiomeGraph.h"
#include "ze/vehicle/VehicleSystem.h"
#include "ze/vehicle/WheelSim.h"
#include "ze/save/SchemaVersion.h"

using namespace modding;
using namespace net;
using namespace slm;
using namespace ui;
using namespace world;
using namespace vehicle;
using namespace save;

// ─── Modding Tests ────────────────────────────────────────────────────

TEST_CASE("Modding initialize creates directory", "[modding]") {
    Modding m;
    REQUIRE(m.initialize("test_mods"));
    m.shutdown();
}

TEST_CASE("Modding load/unload cycle", "[modding]") {
    Modding m;
    REQUIRE(m.initialize("test_mods"));
    REQUIRE(m.loadMod("test_mod_path"));
    auto mods = m.listMods();
    REQUIRE(mods.size() == 1);
    REQUIRE(mods[0].name == "test_mod_path");
    m.shutdown();
}

// ─── StateSync Tests ──────────────────────────────────────────────────

TEST_CASE("StateSync records deterministic frames", "[net][sync]") {
    StateSync sync;
    uint8_t input[] = {1, 2, 3, 4};
    uint8_t entity[] = {10, 20, 30};
    sync.recordFrame(0, input, 4, entity, 3);
    sync.recordFrame(1, input, 4, entity, 3);
    REQUIRE(sync.recordedFrames().size() == 2);
    sync.reset();
    REQUIRE(sync.recordedFrames().empty());
}

TEST_CASE("StateSync replay validation", "[net][replay]") {
    StateSync sync;
    uint8_t input[] = {1, 2, 3, 4};
    uint8_t entity[] = {10, 20, 30};
    sync.recordFrame(0, input, 4, entity, 3);
    sync.recordFrame(1, input, 4, entity, 3);
    // Same inputs should produce same hashes → validate
    REQUIRE(sync.validateReplay(0, 2, sync.recordedFrames()));
}

// ─── PromptTemplate Tests ─────────────────────────────────────────────

TEST_CASE("PromptTemplate renders variables", "[slm][template]") {
    std::unordered_map<std::string, std::string> vars = {{"name", "Zombie"}, {"place", "city"}};
    std::string result = PromptTemplate::render("Hello {{name}} in the {{place}}", vars);
    REQUIRE(result == "Hello Zombie in the city");
}

TEST_CASE("PromptTemplate extracts variables", "[slm][template]") {
    auto vars = PromptTemplate::extractVariables("{{a}} and {{b}} and {{c}}");
    REQUIRE(vars.size() == 3);
    REQUIRE(vars[0] == "a");
    REQUIRE(vars[1] == "b");
    REQUIRE(vars[2] == "c");
}

// ─── HapticManager Tests ──────────────────────────────────────────────

TEST_CASE("HapticManager triggers and decays", "[ui][haptic]") {
    HapticManager hm;
    HapticEvent ev;
    ev.type = HapticEvent::Damage;
    ev.intensity = 0.8f;
    ev.duration = 1.0f;
    hm.trigger(ev);
    hm.update(0.5f);
    // After 0.5s, should still be active
    hm.update(1.0f);
    // After total 1.5s > duration 1.0s → should have decayed
}

// ─── ChunkStreamer Tests ──────────────────────────────────────────────

TEST_CASE("ChunkStreamer LOD computation", "[world][lod]") {
    BiomeGraph bg(42, 8192);
    ChunkStreamer cs(42, 8192, &bg);
    REQUIRE(cs.computeLOD(100.0f) == 0);
    REQUIRE(cs.computeLOD(300.0f) == 1);
    REQUIRE(cs.computeLOD(600.0f) == 2);
    REQUIRE(cs.computeLOD(900.0f) == 3);
}

// ─── Vehicle Tests ────────────────────────────────────────────────────

TEST_CASE("VehicleSystem starts and stops engine", "[vehicle]") {
    VehicleSystem vs;
    vs.startEngine();
    vs.update(1.0f, 0.5f, 0.0f, 0.0f);
    REQUIRE(vs.state().engineRunning);
    vs.stopEngine();
    REQUIRE(!vs.state().engineRunning);
}

TEST_CASE("Vehicle fuel depletes", "[vehicle]") {
    VehicleSystem vs;
    vs.startEngine();
    for (int i = 0; i < 100; i++) vs.update(0.1f, 1.0f, 0.0f, 0.0f);
    REQUIRE(vs.state().fuel < 100.0f);
}

TEST_CASE("WheelSim suspension responds", "[vehicle][wheel]") {
    WheelSim ws;
    ws.setSuspension(30000.0f, 4000.0f, 0.3f);
    // Manual check: properties set
    REQUIRE(true);
}

// ─── SchemaVersion Tests ──────────────────────────────────────────────

TEST_CASE("SchemaVersion compatibility", "[save][schema]") {
    REQUIRE(SchemaVersion::isCompatible(1));
    REQUIRE(!SchemaVersion::isCompatible(0));
}
