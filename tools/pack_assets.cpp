#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

// Include the encryption utilities
#include "../engine/fumbo/utils/assetpack.hpp"
#include "../engine/fumbo/utils/crypto.hpp"

bool verbose = false;

namespace fs = std::filesystem;

struct FileEntry {
  std::string relativePath;
  std::vector<uint8_t> data;
};

void PrintUsage(const char *programName) {
  std::cout << programName << ": no operation specified (use -h for help)";
}

void PrintHelp(const char *programName) {

  std::cout << "Usage: " << programName
            << " <assets_directory> <output_pack_file> [extensions] "
               "[options]\n\n";
  std::cout << "Options: " << "-v, --verbose\n"
            << "         " << "    Verbose program action(s).\n"
            << "         " << "-h, --help\n"
            << "         " << "    Showing this screen.\n\n";
  std::cout << "Example: " << programName
            << " assets/ images.fpk png,jpg,jpeg\n";
}

bool CollectFiles(const fs::path &rootDir, std::vector<FileEntry> &files,
                  const std::vector<std::string> &extensions) {
  if (!fs::exists(rootDir) || !fs::is_directory(rootDir)) {
    std::cerr << "Error: Directory does not exist: " << rootDir << "\n";
    return false;
  }

  for (const auto &entry : fs::recursive_directory_iterator(rootDir)) {
    if (entry.is_regular_file()) {
      // Filter by extension if specified
      if (!extensions.empty()) {
        std::string ext = entry.path().extension().string();
        if (!ext.empty() && ext[0] == '.')
          ext = ext.substr(1); // Remove leading dot

        bool matches = false;
        for (const auto &allowedExt : extensions) {
          if (ext == allowedExt) {
            matches = true;
            break;
          }
        }
        if (!matches)
          continue; // Skip this file
      }

      FileEntry fileEntry;
      // Include the root directory name in the path (e.g.,
      // "assets/ui/button.png")
      fileEntry.relativePath =
          fs::relative(entry.path(), rootDir.parent_path()).string();

      // Read file data
      std::ifstream file(entry.path(), std::ios::binary | std::ios::ate);
      if (!file.is_open()) {
        std::cerr << "Warning: Could not open file: " << entry.path() << "\n";
        continue;
      }

      std::streamsize size = file.tellg();
      file.seekg(0, std::ios::beg);

      fileEntry.data.resize(size);
      if (!file.read(reinterpret_cast<char *>(fileEntry.data.data()), size)) {
        std::cerr << "Warning: Could not read file: " << entry.path() << "\n";
        continue;
      }

      if (verbose)
        std::cout << "  Added: " << fileEntry.relativePath << " (" << size
                  << " bytes)\n";
      files.push_back(
          fileEntry); // Copy instead of move to preserve relativePath
    }
  }

  return !files.empty();
}

bool WritePack(const std::string &outputPath,
               const std::vector<FileEntry> &files) {
  std::ofstream out(outputPath, std::ios::binary);
  if (!out.is_open()) {
    std::cerr << "Error: Could not create output file: " << outputPath << "\n";
    return false;
  }

  // Write header
  Fumbo::Assets::PackHeader header;
  header.magic = Fumbo::Assets::PACK_MAGIC;
  header.version = Fumbo::Assets::PACK_VERSION;
  header.fileCount = static_cast<uint32_t>(files.size());

  out.write(reinterpret_cast<const char *>(&header), sizeof(header));

  // Calculate offsets and write entries
  uint64_t currentOffset = sizeof(Fumbo::Assets::PackHeader) +
                           (sizeof(Fumbo::Assets::PackEntry) * files.size());

  std::vector<Fumbo::Assets::PackEntry> entries;
  for (const auto &file : files) {
    Fumbo::Assets::PackEntry entry;
    entry.nameHash = Fumbo::Assets::HashString(file.relativePath);
    entry.offset = currentOffset;
    entry.size = file.data.size();
    entry.originalSize = file.data.size();

    // Copy filename (truncate if too long)
    std::strncpy(entry.filename, file.relativePath.c_str(),
                 sizeof(entry.filename) - 1);
    entry.filename[sizeof(entry.filename) - 1] = '\0';

    entries.push_back(entry);
    currentOffset += file.data.size();
  }

  // Write all entries
  for (const auto &entry : entries) {
    out.write(reinterpret_cast<const char *>(&entry), sizeof(entry));
  }

  // Write encrypted file data
  for (const auto &file : files) {
    // Encrypt the data
    std::vector<uint8_t> encrypted =
        Fumbo::Crypto::EncryptBuffer(file.data.data(), file.data.size());
    out.write(reinterpret_cast<const char *>(encrypted.data()),
              encrypted.size());
  }

  out.close();
  return true;
}

int main(int argc, char *argv[]) {
  if (argc >= 2) {
    if (argv[1] != NULL && argv[1][0] == '-' && argv[1][1] != '\0') {
      if (argv[1][1] == 'h' || strcmp(argv[1], "--help") == 0) {
        PrintHelp(argv[0]);
        return 0;
      } else {
        PrintUsage(argv[0]);
        return 0;
      }
    }
  } else {
    PrintUsage(argv[0]);
    return 0;
  }

  if (argc >= 5) {
    for (int i = 1; i < argc; i++) {
      if (argv[i] != NULL && argv[i][0] == '-' && argv[i][1] != '\0') {
        if (argv[i][1] == 'v' || strcmp(argv[i], "--verbose")) {
          verbose = true;
        }
      }
    }
  }
  std::string assetsDir = argv[1];
  std::string outputPack = argv[2];

  // Parse optional extensions filter
  std::vector<std::string> extensions;
  if (argc >= 4) {
    std::string extList = argv[3];
    size_t start = 0;
    size_t end = extList.find(',');
    while (end != std::string::npos) {
      extensions.push_back(extList.substr(start, end - start));
      start = end + 1;
      end = extList.find(',', start);
    }
    extensions.push_back(extList.substr(start));
  }

  if (verbose) {
    std::cout << "\nAsset Packer\n";
    std::cout << "============\n";
    std::cout << "Input directory: " << assetsDir << "\n";
    std::cout << "Output pack: " << outputPack << "\n";
    if (!extensions.empty()) {
      std::cout << "Extensions filter: ";
      for (size_t i = 0; i < extensions.size(); ++i) {
        std::cout << extensions[i];
        if (i < extensions.size() - 1)
          std::cout << ", ";
      }
      std::cout << "\n";
    }
    std::cout << "\n";
  }

  // Collect files
  std::vector<FileEntry> files;
  if (verbose)
    std::cout << "Collecting files...\n";
  if (!CollectFiles(assetsDir, files, extensions)) {
    std::cerr << "Error: No files found or could not read directory\n\n";
    return 1;
  }

  if (verbose)
    std::cout << "\nFound " << files.size() << " files\n\n";

  // Write pack
  if (verbose)
    std::cout << "Writing pack file...\n";
  if (!WritePack(outputPack, files)) {
    return 1;
  }

  if (verbose)
    std::cout << "\nSuccess! Created: " << outputPack << "\n\n";
  return 0;
}
