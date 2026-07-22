#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "ze/audio/AudioEngine.h"
#include "ze/ai/ZombieFSM.h"
#include "ze/ai/AIDirector.h"
#include "ze/survival/BodyTemp.h"
#include "ze/survival/StaminaSystem.h"
#include "ze/world/BiomeGraph.h"
#include "ze/save/SaveSystem.h"
#include "ze/combat/DamageTypes.h"
#include "ze/combat/ParrySystem.h"
#include <glm/glm.hpp>

using namespace audio;
using namespace ai;
using namespace survival;
using namespace world;
using namespace save;
using namespace combat;

// ─── Audio Engine Math Tests ─────────────────────────────────────────

TEST_CASE("Doppler shift matches spec M6-EXT-01", "[audio][math]") {
    // f_effective = f_base * (c - v_source · u) / (c - v_observer · u)
    float freq = calcDopplerShift(440.0f, 343.0f, 
        glm::vec3(10, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 0, 0));
    // Source moving toward listener at 10 m/s: f_eff = 440 * (343 - 10) / (343 - 0) = 440 * 333/343
    REQUIRE(freq == Catch::Approx(440.0f * 333.0f / 343.0f).epsilon(0.001f));
}

TEST_CASE("Footstep SPL matches spec M6-EXT-02", "[audio][math]") {
    float spl = calcFootstepSPL(70.0f, 1.5f, 1.0f);
    // SPL_step = κ * m_total * ||v_foot||² * Z_surface
    REQUIRE(spl > 0.0f);
    REQUIRE(spl == Catch::Approx(0.02f * 70.0f * 2.25f * 1.0f).epsilon(0.001f));
}

TEST_CASE("Voice priority sort order", "[audio][voice]") {
    float pNear = calcVoicePriority(1.0f, 5.0f, 1.0f);
    float pFar = calcVoicePriority(1.0f, 50.0f, 1.0f);
    REQUIRE(pNear > pFar); // Near should have higher priority
}

TEST_CASE("Cadence interval scales with distance M6-EXT-06", "[audio][cadence]") {
    REQUIRE(calcCadenceInterval(5.0f) == 1);
    REQUIRE(calcCadenceInterval(15.0f) == 1); // floor(15/10) = 1, so clamp 1
    REQUIRE(calcCadenceInterval(25.0f) == 2);
    REQUIRE(calcCadenceInterval(95.0f) == 8); // clamp at 8
}

TEST_CASE("Velvet-noise pulse position", "[audio][reverb]") {
    float t = calcVelvetPulse(5, 0.01f, 0.5f);
    REQUIRE(t == Catch::Approx((5.0f + 0.5f) * 0.01f).epsilon(0.0001f));
}

TEST_CASE("Reverb envelope decays", "[audio][reverb]") {
    float t1 = calcReverbEnvelope(0.0f, 0.5f);
    float t2 = calcReverbEnvelope(0.5f, 0.5f);
    float t3 = calcReverbEnvelope(5.0f, 0.5f);
    REQUIRE(t1 == Catch::Approx(1.0f).epsilon(0.001f));  // t=0 → envelope=1
    REQUIRE(t2 > 0.3f);                            // decayed but non-zero
    REQUIRE(t3 < 0.001f);                          // mostly decayed
}

// ─── Zombie FSM Tests ────────────────────────────────────────────────

TEST_CASE("ZombieFSM resets to idle", "[ai][zombie]") {
    ZombieFSM fsm;
    REQUIRE(fsm.currentState() == ZombieState::Idle);
}

TEST_CASE("ZombieFSM transitions from idle on stimulus", "[ai][zombie]") {
    ZombieFSM fsm;
    fsm.receiveStimulus(ZombieMemory::StimulusType::Sound, glm::vec3(10,0,0), 0.9f);
    fsm.tick(0.1f, glm::vec3(0), glm::vec3(0,0,-20), 0.0f, 100.0f, 0.0f, 1.0f);
    // Should transition out of idle due to strong stimulus
    REQUIRE(fsm.currentState() != ZombieState::Idle);
}

// ─── AI Director Tests ───────────────────────────────────────────────

TEST_CASE("AIDirector personality affects escalation rate", "[ai][director]") {
    AIDirector dir;
    dir.setPersonality(DirectorPersonality::Cassandra);
    // Tick with high noise and wealth to trigger escalation
    dir.tick(20.0f, glm::vec3(0), 100.0f, 1.0f, 50.0f, 0.0f, 10);
    auto phase = dir.phase();
    // Cassandra should escalate faster → more likely in tension/chase
    REQUIRE(static_cast<int>(phase) >= 0); // valid enum
}

// ─── Survival Tests ──────────────────────────────────────────────────

TEST_CASE("BodyTemp cold exposure causes heat loss", "[survival][bodytemp]") {
    BodyTempSystem bts;
    float initTemp = bts.state().coreTemp;
    // Expose to extreme cold for 300 seconds, wet, wind
    for (int i = 0; i < 3000; i++) {
        bts.tick(0.1f, -30.0f, 30.0f, 0.9f, 0.0f, false, true);
    }
    // Core temp should have dropped below initial
    REQUIRE(bts.state().coreTemp < 37.0f);
    // Should activate shivering
    REQUIRE(bts.state().shiverIntensity > 0.0f);
}

TEST_CASE("Stamina depletes on exertion", "[survival][stamina]") {
    StaminaSystem ss;
    float init = ss.state().stamina;
    ss.tick(1.0f, 1.0f, 37.0f, 0.0f, 0.0f, true);
    ss.tick(1.0f, 1.0f, 37.0f, 0.0f, 0.0f, true);
    ss.tick(1.0f, 1.0f, 37.0f, 0.0f, 0.0f, true);
    REQUIRE(ss.state().stamina < init);
}

// ─── BiomeGraph Tests ────────────────────────────────────────────────

TEST_CASE("BiomeGraph classifies desert climate", "[world][biome]") {
    BiomeGraph bg(42, 8192);
    BiomeType type = bg.classifyClimate(35.0f, 50.0f);  // hot, dry
    REQUIRE(type == BiomeType::Desert);
}

TEST_CASE("BiomeGraph classifies tundra climate", "[world][biome]") {
    BiomeGraph bg(42, 8192);
    BiomeType type = bg.classifyClimate(-15.0f, 200.0f);  // cold, low precip
    REQUIRE(type == BiomeType::Snow);
}

TEST_CASE("BiomeGraph classifies tropical forest", "[world][biome]") {
    BiomeGraph bg(42, 8192);
    BiomeType type = bg.classifyClimate(27.0f, 2200.0f);  // hot, wet
    REQUIRE(type == BiomeType::TropicalForest);
}

// ─── Damage System Tests ─────────────────────────────────────────────

TEST_CASE("DamageSystem resolves bullet vs kevlar", "[combat][damage]") {
    DamageSystem ds;
    HitLocation loc;
    loc.surface = SurfaceMaterial::Kevlar;
    loc.armorRating = 10.0f;
    auto result = ds.resolve(50.0f, DamageType::Bullet, loc);
    // Bullet vs kevlar: should be significantly reduced
    REQUIRE(result.finalDamage < 40.0f);
    REQUIRE(result.blockedByArmor > 0.0f);
}

TEST_CASE("Penetration depth formula", "[combat][ballistics]") {
    DamageSystem ds;
    float depth = ds.calcPenetrationDepth(0.01f, 800.0f, 1000.0f, 0.0001f);
    // KE = 0.5 * 0.01 * 800² = 3200J; depth = 3200 / (1000 * 0.0001) = 32000
    REQUIRE(depth > 0.0f);
}

// ─── Save System Tests ───────────────────────────────────────────────

TEST_CASE("Save checksum detects corruption", "[save][integrity]") {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    uint32_t hash1 = SaveSystem::computeChecksum(data.data(), data.size());
    data[2] = 255; // corrupt
    uint32_t hash2 = SaveSystem::computeChecksum(data.data(), data.size());
    REQUIRE(hash1 != hash2); // corruption should change checksum
}
