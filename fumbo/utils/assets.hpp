#pragma once
#include "assetpack.hpp"
#include "raylib.h"
#include <memory>
#include <string>

namespace Fumbo {
namespace Assets {
// Add asset packs to use (call for each pack file at startup)
void AddAssetPack(const std::string &packPath);

// Get all asset packs
const std::vector<std::unique_ptr<AssetPack>> &GetAssetPacks();

// Asset Loading Wrappers (automatically check pack first)
Texture2D LoadTexture(const std::string &fileName);
Image LoadImage(const std::string &fileName);
Font LoadFont(const std::string &fileName, int fontSize);
Sound LoadSound(const std::string &fileName);
Music LoadMusic(const std::string &fileName);
Texture2D CheckedTexture();
Image CheckedImage();
} // namespace Assets
} // namespace Fumbo
