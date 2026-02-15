#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace Fumbo {
namespace Crypto {

// Simple encryption key
constexpr uint8_t ENCRYPTION_KEY[] = {0x49, 0x4c, 0x4f, 0x56, 0x45, 0x43,
                                      0x55, 0x4e, 0x4e, 0x59, 0x55, 0x4f,
                                      0x4f, 0x48, 0x48, 0x48

};

constexpr size_t KEY_SIZE = sizeof(ENCRYPTION_KEY);

// Encrypt data using XOR with position-based key scrambling
inline void EncryptData(uint8_t *data, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    // Use key position based on data position, with additional scrambling
    uint8_t keyByte = ENCRYPTION_KEY[i % KEY_SIZE];
    uint8_t scramble = static_cast<uint8_t>((i * 7 + 13) & 0xFF);
    data[i] ^= keyByte ^ scramble;
  }
}

// Decrypt data (same operation as encrypt for XOR)
inline void DecryptData(uint8_t *data, size_t size) {
  EncryptData(data, size); // XOR is symmetric
}

// Encrypt and return new buffer
inline std::vector<uint8_t> EncryptBuffer(const uint8_t *data, size_t size) {
  std::vector<uint8_t> encrypted(data, data + size);
  EncryptData(encrypted.data(), encrypted.size());
  return encrypted;
}

// Decrypt and return new buffer
inline std::vector<uint8_t> DecryptBuffer(const uint8_t *data, size_t size) {
  std::vector<uint8_t> decrypted(data, data + size);
  DecryptData(decrypted.data(), decrypted.size());
  return decrypted;
}

} // namespace Crypto
} // namespace Fumbo
