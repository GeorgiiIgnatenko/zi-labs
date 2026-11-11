#include "signature_files.hpp"
#include "../course_files/md5.hpp"

#include <array>
#include <climits>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace signature_files {
namespace {
static_assert(CHAR_BIT == 8, "MD5 требует 8-битные байты.");

void check_new_path(const std::filesystem::path& path) {
    if (std::filesystem::exists(path) || std::filesystem::is_symlink(path)) {
        throw std::runtime_error("Выходной путь уже существует: " + path.string());
    }
}

std::ifstream open_input(const std::filesystem::path& path) {
    if (!std::filesystem::is_regular_file(path)) {
        throw std::runtime_error("Нужен существующий обычный файл: " + path.string());
    }
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Не удалось прочитать файл: " + path.string());
    }
    return input;
}

void check_end(std::istream& input) {
    input >> std::ws;
    if (input.bad() || !input.eof()) {
        throw std::runtime_error("Лишние данные или ошибка чтения формата.");
    }
}

void write_new_file(const std::filesystem::path& path, const std::string& text, bool secret = false) {
    check_new_path(path);
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Не удалось создать файл: " + path.string());
    }
    try {
        if (secret) {
            std::filesystem::permissions(path, std::filesystem::perms::owner_read |
                                              std::filesystem::perms::owner_write);
        }
        output << text;
        output.close();
        if (!output) {
            throw std::runtime_error("Ошибка записи: " + path.string());
        }
    } catch (...) {
        output.close();
        std::error_code ignored;
        std::filesystem::remove(path, ignored);
        throw;
    }
}

std::string key_fields(const crypto::SignaturePublicKey& key) {
    std::ostringstream text;
    text << algorithm_name(key.algorithm) << ' ' << key.p << ' ' << key.q << ' '
         << key.g << ' ' << key.y;
    return text.str();
}

crypto::SignaturePublicKey read_key_fields(std::istream& input, const std::string& magic,
                                         crypto::SignatureAlgorithm algorithm) {
    std::string actual_magic;
    std::string name;
    crypto::SignaturePublicKey key{algorithm, 0, 0, 0, 0};
    if (!(input >> actual_magic >> name >> key.p >> key.q >> key.g >> key.y) ||
        actual_magic != magic || name != algorithm_name(algorithm)) {
        throw std::runtime_error("Неверный формат ключа или ключ от другой лабораторной.");
    }
    crypto::validate_signature_public_key(key);
    return key;
}
} // namespace

std::string algorithm_name(crypto::SignatureAlgorithm algorithm) {
    switch (algorithm) {
        case crypto::SignatureAlgorithm::elgamal: return "ELGAMAL";
        case crypto::SignatureAlgorithm::gost94: return "GOST94";
        case crypto::SignatureAlgorithm::dsa: return "DSA";
    }
    throw std::invalid_argument("Неизвестный алгоритм подписи.");
}

std::string md5_file(const std::filesystem::path& path) {
    auto input = open_input(path);
    MD5 hash;
    std::array<char, 65536> buffer{};
    // binary + gcount сохраняют нули, переводы строк и неполный последний блок.
    while (input.read(buffer.data(), static_cast<std::streamsize>(buffer.size())) || input.gcount() > 0) {
        hash.update(buffer.data(), static_cast<MD5::size_type>(input.gcount()));
    }
    if (!input.eof() || input.bad()) {
        throw std::runtime_error("Ошибка чтения исходного файла.");
    }
    return hash.finalize().hexdigest();
}

long long hash_to_number(const std::string& digest, const crypto::SignaturePublicKey& key) {
    crypto::validate_signature_public_key(key);
    if (digest.size() != 32 || digest.find_first_not_of("0123456789abcdef") != std::string::npos) {
        throw std::invalid_argument("Нужны 32 строчные шестнадцатеричные цифры MD5.");
    }
    const bool elgamal = key.algorithm == crypto::SignatureAlgorithm::elgamal;
    const auto modulus = elgamal ? key.p - 2 : key.q;
    long long h = 0;
    for (char digit : digest) {
        const int value = digit <= '9' ? digit - '0' : digit - 'a' + 10;
        h = (h * 16 + value) % modulus;
    }
    if (elgamal) {
        return h + 2; // Диапазон 1<h<p из лекции 8.
    }
    return key.algorithm == crypto::SignatureAlgorithm::gost94 && h == 0 ? 1 : h;
}

void save_key_pair(const std::filesystem::path& private_path, const std::filesystem::path& public_path,
                   const crypto::SignatureKeyPair& key) {
    crypto::validate_signature_key_pair(key);
    check_new_path(private_path);
    check_new_path(public_path);
    if (std::filesystem::weakly_canonical(private_path) == std::filesystem::weakly_canonical(public_path)) {
        throw std::runtime_error("Для секретного и открытого ключа нужны разные пути.");
    }
    const auto fields = key_fields(key.public_key);
    write_new_file(private_path, "ZISIG-PRIVATE-1 " + fields + ' ' + std::to_string(key.x) + '\n', true);
    try {
        write_new_file(public_path, "ZISIG-PUBLIC-1 " + fields + '\n');
    } catch (...) {
        std::error_code ignored;
        std::filesystem::remove(private_path, ignored);
        throw;
    }
}

crypto::SignatureKeyPair load_private_key(const std::filesystem::path& path, crypto::SignatureAlgorithm algorithm) {
    auto input = open_input(path);
    const auto pub = read_key_fields(input, "ZISIG-PRIVATE-1", algorithm);
    long long x;
    if (!(input >> x)) {
        throw std::runtime_error("В секретном ключе отсутствует x.");
    }
    check_end(input);
    const crypto::SignatureKeyPair key{pub, x};
    crypto::validate_signature_key_pair(key);
    return key;
}

crypto::SignaturePublicKey load_public_key(const std::filesystem::path& path, crypto::SignatureAlgorithm algorithm) {
    auto input = open_input(path);
    const auto key = read_key_fields(input, "ZISIG-PUBLIC-1", algorithm);
    check_end(input);
    return key;
}

FileSignatureResult sign_file(const std::filesystem::path& source, const std::filesystem::path& target,
                              const crypto::SignatureKeyPair& key) {
    crypto::validate_signature_key_pair(key);
    check_new_path(target);
    const auto digest = md5_file(source);
    const auto h = hash_to_number(digest, key.public_key);
    const auto signature = crypto::sign_hash_random(h, key);
    std::ostringstream text;
    text << "ZISIG-1 " << algorithm_name(key.public_key.algorithm) << " MD5-HEX-1\n"
         << signature.r << ' ' << signature.s << '\n';
    write_new_file(target, text.str());
    return {digest, h, signature};
}

bool verify_file(const std::filesystem::path& source, const std::filesystem::path& signature_path,
                 const crypto::SignaturePublicKey& key) {
    crypto::validate_signature_public_key(key);
    auto input = open_input(signature_path);
    std::string magic, algorithm, hash_format;
    crypto::Signature signature{};
    if (!(input >> magic >> algorithm >> hash_format >> signature.r >> signature.s) ||
        magic != "ZISIG-1" || algorithm != algorithm_name(key.algorithm) || hash_format != "MD5-HEX-1") {
        throw std::runtime_error("Неверный формат подписи или другой алгоритм/хеш.");
    }
    check_end(input);
    // Хеш всегда вычисляется заново по проверяемому файлу.
    const auto h = hash_to_number(md5_file(source), key);
    return crypto::verify_hash(h, key, signature);
}
} // namespace signature_files
