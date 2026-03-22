#include "../../fumbo.hpp"
#include "../../tools/crypto.hpp"
#include "raylib.h"
#include <cstring>

namespace Fumbo {
namespace Assets {

bool AssetPack::Load(const std::string &packPath) {
  int dataSize = 0;
  unsigned char *fileData = LoadFileData(packPath.c_str(), &dataSize);

  if (fileData == nullptr) {
    TraceLog(LOG_ERROR, "[AssetPack] Failed to load pack file data: %s",
             packPath.c_str());
    return false;
  }

  // Read header
  if (dataSize < sizeof(PackHeader)) {
    TraceLog(LOG_ERROR, "[AssetPack] Pack file too small");
    UnloadFileData(fileData);
    return false;
  }

  PackHeader header;
  memcpy(&header, fileData, sizeof(PackHeader));

  if (header.magic != PACK_MAGIC) {
    TraceLog(LOG_ERROR, "[AssetPack] Invalid pack file magic number");
    UnloadFileData(fileData);
    return false;
  }

  if (header.version != PACK_VERSION) {
    TraceLog(LOG_ERROR, "[AssetPack] Unsupported pack version: %u",
             header.version);
    UnloadFileData(fileData);
    return false;
  }

  // Read all entries
  entries.clear();

  size_t offset = sizeof(PackHeader);
  for (uint32_t i = 0; i < header.fileCount; ++i) {
    if (offset + sizeof(PackEntry) > dataSize) {
      TraceLog(LOG_ERROR, "[AssetPack] Pack file malformed, unexpected EOF");
      UnloadFileData(fileData);
      return false;
    }
    PackEntry entry;
    memcpy(&entry, fileData + offset, sizeof(PackEntry));
    entries[entry.nameHash] = entry;
    offset += sizeof(PackEntry);
  }

  UnloadFileData(fileData);

  packFilePath = packPath;
  loaded = true;

  TraceLog(LOG_INFO, "[AssetPack] Loaded pack with %u files: %s",
           header.fileCount, packPath.c_str());
  return true;
}

bool AssetPack::HasAsset(const std::string &assetPath) const {
  if (!loaded)
    return false;
  uint64_t hash = HashString(assetPath);
  bool found = entries.find(hash) != entries.end();
  return found;
}

std::vector<uint8_t> AssetPack::LoadAsset(const std::string &assetPath) const {
  if (!loaded) {
    TraceLog(LOG_ERROR, "[AssetPack] Pack not loaded");
    return {};
  }

  uint64_t hash = HashString(assetPath);
  auto it = entries.find(hash);
  if (it == entries.end()) {
    TraceLog(LOG_WARNING, "[AssetPack] Asset not found in pack: %s",
             assetPath.c_str());
    return {};
  }

  const PackEntry &entry = it->second;

  // Open pack file and seek to asset
  int dataSize = 0;
  unsigned char *fileData = LoadFileData(packFilePath.c_str(), &dataSize);

  if (fileData == nullptr) {
    TraceLog(LOG_ERROR,
             "[AssetPack] Failed to load pack file data for reading");
    return {};
  }

  if (entry.offset + entry.size > dataSize) {
    TraceLog(LOG_ERROR, "[AssetPack] Pack file malformed, asset out of bounds");
    UnloadFileData(fileData);
    return {};
  }

  // Read encrypted data
  std::vector<uint8_t> encryptedData(entry.size);
  memcpy(encryptedData.data(), fileData + entry.offset, entry.size);
  UnloadFileData(fileData);

  // Decrypt
  Crypto::DecryptData(encryptedData.data(), encryptedData.size());

  return encryptedData;
}

size_t AssetPack::GetAssetSize(const std::string &assetPath) const {
  if (!loaded)
    return 0;
  uint64_t hash = HashString(assetPath);
  auto it = entries.find(hash);
  if (it == entries.end())
    return 0;
  return it->second.originalSize;
}

void AssetPack::Unload() {
  entries.clear();
  packFilePath.clear();
  loaded = false;
}

} // namespace Assets
} // namespace Fumbo
