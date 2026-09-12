#include "ze/render/AssetCooker.h"
#include <fstream>
#include <algorithm>
#include <cstring>

namespace ze::render {

uint32_t AssetCooker::computeCRC32(const uint8_t* data, size_t size) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            crc = (crc >> 1) ^ (0xEDB88320u & (-(int32_t)(crc & 1)));
        }
    }
    return ~crc;
}

size_t AssetCooker::calculateBC7Size(uint32_t width, uint32_t height, uint32_t mipLevels) {
    size_t totalBytes = 0;
    uint32_t w = width;
    uint32_t h = height;

    for (uint32_t m = 0; m < mipLevels; ++m) {
        uint32_t blocksW = (std::max(1u, w) + 3u) / 4u;
        uint32_t blocksH = (std::max(1u, h) + 3u) / 4u;
        totalBytes += static_cast<size_t>(blocksW) * blocksH * 16u; // 16 bytes per 4x4 block in BC7

        w = std::max(1u, w / 2u);
        h = std::max(1u, h / 2u);
    }
    return totalBytes;
}

size_t AssetCooker::calculateBC5Size(uint32_t width, uint32_t height, uint32_t mipLevels) {
    size_t totalBytes = 0;
    uint32_t w = width;
    uint32_t h = height;

    for (uint32_t m = 0; m < mipLevels; ++m) {
        uint32_t blocksW = (std::max(1u, w) + 3u) / 4u;
        uint32_t blocksH = (std::max(1u, h) + 3u) / 4u;
        totalBytes += static_cast<size_t>(blocksW) * blocksH * 16u; // 16 bytes per 4x4 block in BC5 (8B per channel)

        w = std::max(1u, w / 2u);
        h = std::max(1u, h / 2u);
    }
    return totalBytes;
}

bool AssetCooker::buildPackage(const std::string& outputPath,
                               const std::vector<std::pair<ZePakEntry, std::vector<uint8_t>>>& assets) {
    std::ofstream out(outputPath, std::ios::binary);
    if (!out.is_open()) {
        return false;
    }

    ZePakHeader header;
    header.magic = 0x5A45504B; // 'ZEPK'
    header.version = 1;
    header.entryCount = static_cast<uint32_t>(assets.size());
    header.tableOffset = 0;

    // Reserve space for header
    out.write(reinterpret_cast<const char*>(&header), sizeof(ZePakHeader));

    std::vector<ZePakEntry> updatedEntries;
    updatedEntries.reserve(assets.size());

    // Write asset payloads
    for (const auto& item : assets) {
        ZePakEntry entry = item.first;
        const auto& data = item.second;

        entry.offset = static_cast<uint64_t>(out.tellp());
        entry.size = static_cast<uint64_t>(data.size());
        entry.uncompressedSize = entry.size;
        entry.crc32 = computeCRC32(data.data(), data.size());

        if (!data.empty()) {
            out.write(reinterpret_cast<const char*>(data.data()), data.size());
        }
        updatedEntries.push_back(entry);
    }

    // Write table offset
    header.tableOffset = static_cast<uint64_t>(out.tellp());
    for (const auto& entry : updatedEntries) {
        out.write(reinterpret_cast<const char*>(&entry), sizeof(ZePakEntry));
    }

    // Rewrite header with tableOffset
    out.seekp(0, std::ios::beg);
    out.write(reinterpret_cast<const char*>(&header), sizeof(ZePakHeader));
    out.close();

    return true;
}

bool AssetCooker::readPackageTable(const std::string& packagePath,
                                   ZePakHeader& outHeader,
                                   std::vector<ZePakEntry>& outEntries) {
    std::ifstream in(packagePath, std::ios::binary);
    if (!in.is_open()) {
        return false;
    }

    in.read(reinterpret_cast<char*>(&outHeader), sizeof(ZePakHeader));
    if (outHeader.magic != 0x5A45504B || outHeader.version != 1) {
        return false;
    }

    in.seekg(outHeader.tableOffset, std::ios::beg);
    outEntries.resize(outHeader.entryCount);
    for (uint32_t i = 0; i < outHeader.entryCount; ++i) {
        in.read(reinterpret_cast<char*>(&outEntries[i]), sizeof(ZePakEntry));
    }

    return true;
}

bool AssetCooker::extractEntry(const std::string& packagePath,
                              const ZePakEntry& entry,
                              std::vector<uint8_t>& outData) {
    std::ifstream in(packagePath, std::ios::binary);
    if (!in.is_open()) {
        return false;
    }

    in.seekg(entry.offset, std::ios::beg);
    outData.resize(entry.size);
    in.read(reinterpret_cast<char*>(outData.data()), entry.size);

    uint32_t crc = computeCRC32(outData.data(), outData.size());
    return (crc == entry.crc32);
}

} // namespace ze::render
