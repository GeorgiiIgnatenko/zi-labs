#pragma once

#include "../../common/crypto.hpp"

#include <climits>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace lab04 {

static_assert(CHAR_BIT == 8, "Формат лабораторной рассчитан на 8-битный байт.");

inline void check_paths(const std::filesystem::path& source, const std::filesystem::path& target) {
    if (!std::filesystem::is_regular_file(source)) {
        throw std::runtime_error("Входной путь должен указывать на существующий обычный файл.");
    }
    // Существующие файлы и ссылки не перезаписываем, включая ссылку на оригинал.
    if (std::filesystem::exists(target) || std::filesystem::is_symlink(target)) {
        throw std::runtime_error("Выходной путь уже существует. Укажите новое имя.");
    }
}

inline void check_file_modulus(long long p) {
    if (p <= 255 || p > crypto::max_number || !crypto::is_prime(p)) {
        throw std::invalid_argument("Для всех байтов файла нужно простое 255 < p <= 2000000000.");
    }
}

// Заголовок SHAMIR1 p N, затем N чисел x3. Секретных показателей в файле нет.
inline std::uintmax_t encrypt_file(const std::filesystem::path& source,
                                 const std::filesystem::path& target,
                                 const crypto::ShamirParameters& key) {
    crypto::validate_shamir_parameters(key);
    check_file_modulus(key.p);
    check_paths(source, target);
    const auto size = std::filesystem::file_size(source);
    std::ifstream input(source, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Не удалось открыть исходный файл.");
    }
    std::ofstream output(target, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Не удалось создать зашифрованный файл.");
    }
    try {
        output << "SHAMIR1 " << key.p << ' ' << size << '\n';
        std::uintmax_t count = 0;
        char byte;
        while (input.get(byte)) {
            const auto m = static_cast<unsigned char>(byte);
            const auto x1 = crypto::mod_pow(m, key.c_a, key.p);
            const auto x2 = crypto::mod_pow(x1, key.c_b, key.p);
            const auto x3 = crypto::mod_pow(x2, key.d_a, key.p);
            output << x3 << '\n';
            ++count;
        }
        if (!input.eof() || input.bad() || count != size) {
            throw std::runtime_error("Ошибка чтения или размер исходного файла изменился.");
        }
        output.close();
        if (!output) {
            throw std::runtime_error("Ошибка записи зашифрованного файла.");
        }
        return count;
    } catch (...) {
        output.close();
        std::error_code ignored;
        std::filesystem::remove(target, ignored);
        throw;
    }
}

inline std::uintmax_t decrypt_file(const std::filesystem::path& source,
                                 const std::filesystem::path& target, long long d_b) {
    check_paths(source, target);
    std::ifstream input(source, std::ios::binary);
    std::string magic;
    long long p;
    // Размер читается как строка: отрицательное значение не должно стать unsigned.
    std::string size_text;
    if (!(input >> magic >> p >> size_text) || magic != "SHAMIR1" ||
        size_text.empty() || size_text.find_first_not_of("0123456789") != std::string::npos) {
        throw std::runtime_error("Неверный заголовок SHAMIR1 p N.");
    }
    const auto size = std::stoull(size_text);
    check_file_modulus(p);
    if (d_b <= 2 || d_b >= p - 1 || crypto::extended_gcd(d_b, p - 1).gcd != 1) {
        throw std::invalid_argument("Нужен D_B из (2, p-1), взаимно простой с p-1.");
    }
    std::ofstream output(target, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Не удалось создать восстановленный файл.");
    }
    try {
        for (unsigned long long i = 0; i < size; ++i) {
            long long x3;
            if (!(input >> x3) || x3 < 0 || x3 >= p) {
                throw std::runtime_error("Шифротекст оборван или содержит неверный блок.");
            }
            const auto m = crypto::mod_pow(x3, d_b, p);
            if (m > 255) {
                throw std::runtime_error("Получено не значение байта: неверный ключ или повреждение.");
            }
            const auto byte = static_cast<unsigned char>(m);
            output.write(reinterpret_cast<const char*>(&byte), 1);
        }
        std::string extra;
        if (input >> extra) {
            throw std::runtime_error("В шифротексте есть лишние данные.");
        }
        if (!input.eof() || input.bad()) {
            throw std::runtime_error("Ошибка чтения шифротекста.");
        }
        output.close();
        if (!output) {
            throw std::runtime_error("Ошибка записи восстановленного файла.");
        }
        return size;
    } catch (...) {
        output.close();
        std::error_code ignored;
        std::filesystem::remove(target, ignored);
        throw;
    }
}

} // namespace lab04
