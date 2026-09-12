#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ze::render {

enum class AssetType : uint32_t {
    Unknown = 0,
    TextureBC7,
    TextureBC5,
    MeshVAT,
    PhysicsHull,
    MaterialData
};

#pragma pack(push, 1)
struct ZePakHeader {
    uint32_t magic = 0x5A45504B; // 'ZEPK'
    uint32_t version = 1;
    uint32_t entryCount = 0;
    uint64_t tableOffset = 0;
};

struct ZePakEntry {
    char name[64] = {};
    uint32_t assetType = 0;
    uint64_t offset = 0;
    uint64_t size = 0;
    uint64_t uncompressedSize = 0;
    uint32_t crc32 = 0;
};

struct BC7TextureHeader {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t mipLevels = 1;
    uint32_t vkFormat = 145; // VK_FORMAT_BC7_UNORM_BLOCK
    uint32_t totalBytes = 0;
};

struct BC5TextureHeader {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t mipLevels = 1;
    uint32_t vkFormat = 141; // VK_FORMAT_BC5_UNORM_BLOCK
    uint32_t totalBytes = 0;
};

struct VATMeshHeader {
    uint32_t vertexCount = 0;
    uint32_t frameCount = 0;
    float framesPerSecond = 30.0f;
    float boundsMin[3] = {};
    float boundsMax[3] = {};
    uint32_t posTexWidth = 0;
    uint32_t posTexHeight = 0;
    uint32_t normTexWidth = 0;
    uint32_t normTexHeight = 0;
};

struct JoltPrebakedHullHeader {
    uint32_t shapeType = 1; // 1 = ConvexHull, 2 = Compound
    uint32_t vertexCount = 0;
    uint32_t faceCount = 0;
    float mass = 1.0f;
    float friction = 0.5f;
    float restitution = 0.1f;
    float centerOfMass[3] = {};
};
#pragma pack(pop)

class AssetCooker {
public:
    static size_t calculateBC7Size(uint32_t width, uint32_t height, uint32_t mipLevels = 1);
    static size_t calculateBC5Size(uint32_t width, uint32_t height, uint32_t mipLevels = 1);

    // Build a .zepak archive from in-memory cooked entries
    static bool buildPackage(const std::string& outputPath,
                            const std::vector<std::pair<ZePakEntry, std::vector<uint8_t>>>& assets);

    // Inspect and read a .zepak archive table
    static bool readPackageTable(const std::string& packagePath,
                                ZePakHeader& outHeader,
                                std::vector<ZePakEntry>& outEntries);

    // Helper to extract a single entry by name
    static bool extractEntry(const std::string& packagePath,
                            const ZePakEntry& entry,
                            std::vector<uint8_t>& outData);

    static uint32_t computeCRC32(const uint8_t* data, size_t size);
};

} // namespace ze::render
