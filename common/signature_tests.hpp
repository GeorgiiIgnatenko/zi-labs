#pragma once

// Общие проверки трёх лабораторных; этот файл не входит в рабочие программы.
#include "crypto.hpp"
#include "signature_files.hpp"

#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace signature_tests {
inline void require(bool condition) {
    if (!condition) {
        throw std::runtime_error("Проверка не пройдена.");
    }
}

template<class Function> void rejects(Function function) {
    try {
        function();
    } catch (const std::invalid_argument&) {
        return;
    }
    throw std::runtime_error("Ожидалось отклонение параметров.");
}

// Независимый медленный расчёт для малых модулей, без crypto::mod_pow/inverse.
inline long long power(long long a, long long n, long long p) {
    long long result = 1;
    for (long long i = 0; i < n; ++i) {
        result = result * a % p;
    }
    return result;
}

inline long long inverse(long long a, long long p) {
    for (long long i = 1; i < p; ++i) {
        if (a * i % p == 1) {
            return i;
        }
    }
    throw std::runtime_error("Нет обратного в эталоне.");
}

inline int run(crypto::SignatureAlgorithm algorithm) {
    try {
        using crypto::SignatureAlgorithm;
        const bool elgamal = algorithm == SignatureAlgorithm::elgamal;
        const bool gost = algorithm == SignatureAlgorithm::gost94;
        const auto p = elgamal ? 23 : 67;
        const auto q = elgamal ? 0 : 11;
        const auto g = elgamal ? 5 : 25;
        const auto key = crypto::make_signature_key(algorithm, p, q, g, elgamal ? 7 : 6);
        const auto sample = crypto::sign_hash(3, key, elgamal ? 5 : 8);
        require(sample && sample->r == (elgamal ? 20 : 2) && sample->s == (elgamal ? 21 : (gost ? 3 : 6)));
        require(crypto::verify_hash(3, key.public_key, *sample));
        int cases = 0;
        int retries = 0;
        const auto order = elgamal ? p - 1 : q;
        for (long long x = elgamal ? 2 : 1; x < order; ++x) {
            const auto small_key = crypto::make_signature_key(algorithm, p, q, g, x);
            require(small_key.public_key.y == power(g, x, p));
            for (long long h = elgamal ? 2 : (gost ? 1 : 0); h < (elgamal ? p : q); ++h) {
                for (long long k = elgamal ? 2 : 1; k < order; ++k) {
                    if (elgamal && std::gcd(k, order) != 1) {
                        continue;
                    }
                    const auto r = elgamal ? power(g, k, p) : power(g, k, p) % q;
                    const auto s = elgamal ? ((h - x * r) % order + order) % order * inverse(k, order) % order
                        : (gost ? (k * h + x * r) % q : inverse(k, q) * (h + x * r) % q);
                    const auto actual = crypto::sign_hash(h, small_key, k);
                    if (!elgamal && (r == 0 || s == 0)) {
                        require(!actual);
                        ++retries;
                    } else {
                        require(actual && actual->r == r && actual->s == s);
                        require(crypto::verify_hash(h, small_key.public_key, *actual));
                    }
                    ++cases;
                }
            }
        }
        // Проверяем и ложные подписи: независимое уравнение для всех r,s малого примера.
        int verification_cases = 0;
        for (long long r = -1; r <= p; ++r) {
            for (long long s = -1; s <= p; ++s) {
                bool expected = false;
                if (elgamal && r > 0 && r < p && s >= 0 && s < p - 1) {
                    expected = power(key.public_key.y, r, p) * power(r, s, p) % p == power(g, 3, p);
                } else if (!elgamal && r > 0 && r < q && s > 0 && s < q) {
                    const auto inv = inverse(gost ? 3 : s, q);
                    const auto u1 = (gost ? s : 3) * inv % q;
                    const auto u2 = gost ? (q - r * inv % q) % q : r * inv % q;
                    expected = (power(g, u1, p) * power(key.public_key.y, u2, p) % p) % q == r;
                }
                require(crypto::verify_hash(3, key.public_key, {r, s}) == expected);
                ++verification_cases;
            }
        }
        const auto zeros = signature_files::hash_to_number(std::string(32, '0'), key.public_key);
        require(zeros == (elgamal ? 2 : (gost ? 1 : 0)));
        rejects([&] { signature_files::hash_to_number("bad", key.public_key); });
        rejects([&] { crypto::make_signature_key(algorithm, 21, q, g, 6); });
        rejects([&] { crypto::make_signature_key(algorithm, p, q, 1, 6); });
        rejects([&] { crypto::make_signature_key(algorithm, p, q, g, 0); });
        rejects([&] { crypto::make_signature_key(algorithm, p, q, g, order); });
        rejects([&] { crypto::sign_hash(-1, key, 5); });
        rejects([&] { crypto::sign_hash(elgamal ? p : q, key, 5); });
        rejects([&] { crypto::sign_hash(3, key, 0); });
        rejects([&] { crypto::sign_hash(3, key, order); });
        if (elgamal) {
            rejects([&] { crypto::sign_hash(3, key, 2); });
        } else {
            rejects([&] { crypto::make_signature_key(algorithm, p, 10, g, 6); });
            rejects([&] { crypto::make_signature_key(algorithm, p, 13, g, 6); });
            rejects([&] { crypto::make_signature_key(algorithm, p, q, 2, 6); });
        }
        auto bad_key = key;
        ++bad_key.x;
        rejects([&] { crypto::validate_signature_key_pair(bad_key); });
        auto bad_pub = key.public_key;
        bad_pub.y = 1;
        rejects([&] { crypto::verify_hash(3, bad_pub, *sample); });
        bad_pub = key.public_key;
        bad_pub.p = std::numeric_limits<long long>::max();
        rejects([&] { crypto::verify_hash(3, bad_pub, *sample); });
        require(!crypto::verify_hash(3, key.public_key, {std::numeric_limits<long long>::max(), 1}));
        for (int i = 0; i < 5; ++i) {
            const auto generated = crypto::generate_signature_key(algorithm);
            const auto digest = std::string(32, 'f');
            const auto h = signature_files::hash_to_number(digest, generated.public_key);
            const auto signature = crypto::sign_hash_random(h, generated);
            require(crypto::verify_hash(h, generated.public_key, signature));
            require(generated.public_key.p >= 1'000'000'000);
        }
        // Верхняя граница p: точное простое 1 999 999 973 для Эль-Гамаля.
        if (elgamal) {
            long long root = 2;
            while (!crypto::is_primitive_root(root, 1'999'999'973)) {
                ++root;
            }
            const auto large = crypto::make_signature_key(algorithm, 1'999'999'973, 0, root, 1'999'999'971);
            const auto signature = crypto::sign_hash(1'999'999'972, large, 1'999'999'971);
            require(signature && crypto::verify_hash(1'999'999'972, large.public_key, *signature));
        }
        std::cout << signature_files::algorithm_name(algorithm) << ": " << cases << " расчётов подписи, "
                  << retries << " повторов k, " << verification_cases << " проверок пар r,s; границы и 5 генераций OK.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
} // namespace signature_tests
