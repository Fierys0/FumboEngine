#pragma once
#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace Fumbo {
namespace Assets {

// Asset pack file format
constexpr uint32_t PACK_MAGIC = 0x4B415046;  // "FPAK"
constexpr uint32_t PACK_VERSION = 1;

struct PackHeader {
  uint32_t magic;
  uint32_t version;
  uint32_t fileCount;
};

struct PackEntry {
  uint64_t nameHash;     // Hash of the filename
  uint64_t offset;       // Offset in the pack file
  uint64_t size;         // Size of encrypted data
  uint64_t originalSize; // Size of original data
  char filename[256];    // Original filename for debugging
};

// Normalize path separators (Windows backslash to forward slash)
inline std::string NormalizePath(const std::string& path) {
  std::string normalized = path;
  for (char& c : normalized) {
    if (c == '\\') c = '/';
  }
  return normalized;
}

// Simple hash function for filenames
inline uint64_t HashString(const std::string& str) {
  // Normalize path before hashing to ensure Windows/Linux compatibility
  std::string normalized = NormalizePath(str);
  uint64_t hash = 5381;
  for (char c : normalized) {
    hash = ((hash << 5) + hash) + static_cast<uint64_t>(c);
  }
  return hash;
}

class AssetPack {
public:
  AssetPack() : loaded(false) {}

  // Load a pack file
  bool Load(const std::string& packPath);

  // Check if pack is loaded
  bool IsLoaded() const { return loaded; }

  // Check if asset exists in pack
  bool HasAsset(const std::string& assetPath) const;

  // Load asset data (decrypted)
  std::vector<uint8_t> LoadAsset(const std::string& assetPath) const;

  // Get original size of asset
  size_t GetAssetSize(const std::string& assetPath) const;

  // Unload pack
  void Unload();

private:
  bool loaded;
  std::string packFilePath;
  std::unordered_map<uint64_t, PackEntry> entries;
};

}  // namespace Assets
}  // namespace Fumbo
