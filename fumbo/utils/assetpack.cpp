#include "../../fumbo.hpp"
#include "../../tools/crypto.hpp"
#include "raylib.h"

namespace Fumbo {
namespace Assets {

bool AssetPack::Load(const std::string& packPath) {
  std::ifstream file(packPath, std::ios::binary);
  if (!file.is_open()) {
    TraceLog(LOG_ERROR, "[AssetPack] Failed to open pack file: %s", packPath.c_str());
    return false;
  }

  // Read header
  PackHeader header;
  file.read(reinterpret_cast<char*>(&header), sizeof(PackHeader));

  if (header.magic != PACK_MAGIC) {
    TraceLog(LOG_ERROR, "[AssetPack] Invalid pack file magic number");
    file.close();
    return false;
  }

  if (header.version != PACK_VERSION) {
    TraceLog(LOG_ERROR, "[AssetPack] Unsupported pack version: %u", header.version);
    file.close();
    return false;
  }

  // Read all entries
  entries.clear();
  for (uint32_t i = 0; i < header.fileCount; ++i) {
    PackEntry entry;
    file.read(reinterpret_cast<char*>(&entry), sizeof(PackEntry));
    entries[entry.nameHash] = entry;
  }

  packFilePath = packPath;
  loaded = true;

  TraceLog(LOG_INFO, "[AssetPack] Loaded pack with %u files: %s", header.fileCount, packPath.c_str());
  return true;
}

bool AssetPack::HasAsset(const std::string& assetPath) const {
  if (!loaded) return false;
  uint64_t hash = HashString(assetPath);
  bool found = entries.find(hash) != entries.end();
  return found;
}

std::vector<uint8_t> AssetPack::LoadAsset(const std::string& assetPath) const {
  if (!loaded) {
    TraceLog(LOG_ERROR, "[AssetPack] Pack not loaded");
    return {};
  }

  uint64_t hash = HashString(assetPath);
  auto it = entries.find(hash);
  if (it == entries.end()) {
    TraceLog(LOG_WARNING, "[AssetPack] Asset not found in pack: %s", assetPath.c_str());
    return {};
  }

  const PackEntry& entry = it->second;

  // Open pack file and seek to asset
  std::ifstream file(packFilePath, std::ios::binary);
  if (!file.is_open()) {
    TraceLog(LOG_ERROR, "[AssetPack] Failed to open pack file for reading");
    return {};
  }

  file.seekg(entry.offset);

  // Read encrypted data
  std::vector<uint8_t> encryptedData(entry.size);
  file.read(reinterpret_cast<char*>(encryptedData.data()), entry.size);
  file.close();

  // Decrypt
  Crypto::DecryptData(encryptedData.data(), encryptedData.size());

  return encryptedData;
}

size_t AssetPack::GetAssetSize(const std::string& assetPath) const {
  if (!loaded) return 0;
  uint64_t hash = HashString(assetPath);
  auto it = entries.find(hash);
  if (it == entries.end()) return 0;
  return it->second.originalSize;
}

void AssetPack::Unload() {
  entries.clear();
  packFilePath.clear();
  loaded = false;
}

}  // namespace Assets
}  // namespace Fumbo
