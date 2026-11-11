#pragma once

#include "crypto.hpp"

#include <filesystem>
#include <string>

namespace signature_files {

std::string algorithm_name(crypto::SignatureAlgorithm algorithm);
std::string md5_file(const std::filesystem::path& path);
// MD5 как 32 hex-цифры слева направо; никаких 128-битных целых не требуется.
long long hash_to_number(const std::string& digest, const crypto::SignaturePublicKey& key);
void save_key_pair(const std::filesystem::path& private_path, const std::filesystem::path& public_path,
                   const crypto::SignatureKeyPair& key);
crypto::SignatureKeyPair load_private_key(const std::filesystem::path& path, crypto::SignatureAlgorithm algorithm);
crypto::SignaturePublicKey load_public_key(const std::filesystem::path& path, crypto::SignatureAlgorithm algorithm);

struct FileSignatureResult {
    std::string digest;
    long long h;
    crypto::Signature signature;
};

FileSignatureResult sign_file(const std::filesystem::path& source, const std::filesystem::path& target,
                              const crypto::SignatureKeyPair& key);
bool verify_file(const std::filesystem::path& source, const std::filesystem::path& signature_path,
                 const crypto::SignaturePublicKey& key);

} // namespace signature_files
