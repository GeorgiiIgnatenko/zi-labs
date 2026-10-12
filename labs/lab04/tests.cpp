#include "files.hpp"

#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <numeric>
#include <vector>

void check(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void expect_error(const std::function<void()>& action) {
    try {
        action();
    } catch (const std::exception&) {
        return;
    }
    throw std::runtime_error("Ожидалось отклонение неверных данных");
}

long long slow_power(long long a, long long x, long long p) {
    long long result = 1;
    while (x-- > 0) {
        result = result * a % p;
    }
    return result;
}

void write_file(const std::filesystem::path& path, const std::string& data) {
    std::ofstream file(path, std::ios::binary);
    file.write(data.data(), static_cast<std::streamsize>(data.size()));
    file.close();
    check(static_cast<bool>(file), "Не удалось записать тестовые данные");
}

std::string read_file(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    check(static_cast<bool>(file), "Не удалось прочитать тестовые данные");
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

int main() {
    std::filesystem::path directory;
    try {
        long long inverse_cases = 0;
        for (long long modulus = 2; modulus <= 100; ++modulus) {
            for (long long a = 1; a < modulus; ++a) {
                if (std::gcd(a, modulus) == 1) {
                    const auto d = crypto::mod_inverse(a, modulus);
                    check(d > 0 && d < modulus && a * d % modulus == 1, "Неверное обратное число");
                } else {
                    expect_error([=] { crypto::mod_inverse(a, modulus); });
                }
                ++inverse_cases;
            }
        }
        long long exchanges = 0;
        for (long long p : {5, 7, 11, 13, 17, 19, 23, 29, 31}) {
            for (long long ca = 3; ca < p - 1; ++ca) {
                if (std::gcd(ca, p - 1) != 1) { continue; }
                for (long long cb = 3; cb < p - 1; ++cb) {
                    if (std::gcd(cb, p - 1) != 1) { continue; }
                    const auto key = crypto::make_shamir_parameters(p, ca, cb);
                    for (long long m = 0; m < p; ++m) {
                        const auto s = crypto::shamir_exchange(m, key);
                        check(s.x1 == slow_power(m, ca, p), "Ошибка первого шага");
                        check(s.x2 == slow_power(s.x1, cb, p), "Ошибка второго шага");
                        check(s.x3 == slow_power(s.x2, key.d_a, p), "Ошибка третьего шага");
                        check(s.x4 == slow_power(s.x3, key.d_b, p) && s.x4 == m, "Сообщение не восстановлено");
                        ++exchanges;
                    }
                }
            }
        }
        const auto lecture = crypto::make_shamir_parameters(23, 7, 5);
        const auto steps = crypto::shamir_exchange(10, lecture);
        check(lecture.d_a == 19 && lecture.d_b == 9, "Ключи примера лекции");
        check(steps.x1 == 14 && steps.x2 == 15 && steps.x3 == 19 && steps.x4 == 10, "Пример лекции");
        const auto large = crypto::shamir_exchange(254, crypto::make_shamir_parameters(1'999'999'973, 3, 5));
        check(large.x1 == 16'387'064 && large.x2 == 984'359'941 &&
              large.x3 == 1'227'835'280 && large.x4 == 254, "Независимый большой пример");
        for (int i = 0; i < 8; ++i) {
            const auto key = crypto::generate_shamir_parameters();
            check(key.p >= 1'000'000'000 && key.p <= crypto::max_number && crypto::is_prime(key.p), "Генератор p");
            check(key.c_a * key.d_a % (key.p - 1) == 1 && key.c_b * key.d_b % (key.p - 1) == 1, "Генератор показателей");
            for (long long m : {0LL, 1LL, 255LL, key.p - 1}) {
                check(crypto::shamir_exchange(m, key).x4 == m, "Обмен с генератором");
            }
        }
        for (long long p : {0LL, 3LL, 21LL, 561LL, 2'000'000'001LL, std::numeric_limits<long long>::max()}) {
            expect_error([=] { crypto::make_shamir_parameters(p, 7, 5); });
        }
        for (long long c : {0LL, 1LL, 2LL, 4LL, 11LL, 22LL, std::numeric_limits<long long>::min()}) {
            expect_error([=] { crypto::make_shamir_parameters(23, c, 5); });
            expect_error([=] { crypto::make_shamir_parameters(23, 7, c); });
        }
        expect_error([] { crypto::mod_inverse(1, 1); });
        expect_error([] { crypto::mod_inverse(0, 23); });
        expect_error([] { crypto::mod_inverse(23, 23); });
        expect_error([] { crypto::mod_inverse(1, 2'000'000'001); });
        expect_error([&] { crypto::shamir_exchange(-1, lecture); });
        expect_error([&] { crypto::shamir_exchange(23, lecture); });
        auto broken_key = lecture;
        broken_key.d_a = 1;
        expect_error([&] { crypto::shamir_exchange(10, broken_key); });

        do {
            directory = "build/lab04-test-" + std::to_string(crypto::random_number(1, crypto::max_number));
        } while (!std::filesystem::create_directory(directory));
        std::string all_bytes;
        for (int byte = 0; byte <= 255; ++byte) {
            all_bytes += static_cast<char>(static_cast<unsigned char>(byte));
        }
        const auto small_key = crypto::make_shamir_parameters(257, 3, 5);
        const auto big_key = crypto::make_shamir_parameters(1'999'999'973, 3, 5);
        const std::vector<std::string> contents{"", "A", std::string("\0\xff\r\n", 4),
                                                "Привет, университет!\n", all_bytes, std::string(20'000, 'x')};
        int file_cases = 0;
        for (const auto& key : {small_key, big_key}) {
            for (const auto& data : contents) {
                const auto base = directory / ("файл с пробелом " + std::to_string(file_cases++));
                const auto source = base.string() + ".bin";
                const auto encrypted = base.string() + ".shamir";
                const auto restored = base.string() + ".restored";
                write_file(source, data);
                check(lab04::encrypt_file(source, encrypted, key) == data.size(), "Размер при шифровании");
                check(lab04::decrypt_file(encrypted, restored, key.d_b) == data.size(), "Размер при расшифровании");
                check(read_file(restored) == data && read_file(source) == data, "Байты изменились");
            }
        }
        const auto source = directory / "original.bin";
        const auto encrypted = directory / "original.shamir";
        const auto output = directory / "must-not-exist.bin";
        write_file(source, "A");
        lab04::encrypt_file(source, encrypted, small_key);
        check(read_file(encrypted) == "SHAMIR1 257 1\n132\n", "Формат и шифрование байта A");
        expect_error([&] { lab04::encrypt_file(source, source, small_key); });
        expect_error([&] { lab04::decrypt_file(encrypted, source, small_key.d_b); });
        expect_error([&] { lab04::encrypt_file(directory / "missing", output, small_key); });
        expect_error([&] { lab04::encrypt_file(directory, output, small_key); });
        expect_error([&] { lab04::encrypt_file(source, output, lecture); });
        expect_error([&] { lab04::decrypt_file(encrypted, output, 2); });
        expect_error([&] { lab04::decrypt_file(encrypted, output, 256); });
        check(read_file(source) == "A" && !std::filesystem::exists(output), "Испорчен исходный файл");
        std::filesystem::create_symlink(source.filename(), directory / "link");
        expect_error([&] { lab04::encrypt_file(source, directory / "link", small_key); });
        std::filesystem::create_symlink("missing", directory / "dangling");
        expect_error([&] { lab04::encrypt_file(source, directory / "dangling", small_key); });

        const std::vector<std::string> malformed{
            "", "OTHER 257 1\n132\n", "SHAMIR1 255 0", "SHAMIR1 259 0", "SHAMIR1 257 -1",
            "SHAMIR1 257 99999999999999999999999999", "SHAMIR1 257 1", "SHAMIR1 257 2\n132",
            "SHAMIR1 257 1\n-1", "SHAMIR1 257 1\n257", "SHAMIR1 257 1\n256",
            "SHAMIR1 257 1\n132\n42", "SHAMIR1 257 0\n132", "SHAMIR1 257 1\n1.5",
            "SHAMIR1 257 1\n99999999999999999999999", "SHAMIR1 257 1\nabc"};
        const auto bad_source = directory / "bad.shamir";
        for (const auto& data : malformed) {
            write_file(bad_source, data);
            expect_error([&] { lab04::decrypt_file(bad_source, output, small_key.d_b); });
            check(!std::filesystem::exists(output), "После ошибки остался неполный файл");
        }
        // При большом p заведомо неверный допустимый D_B даёт значение вне байта.
        write_file(bad_source, "SHAMIR1 1000000007 1\n290717726\n");
        expect_error([&] { lab04::decrypt_file(bad_source, output, 3); });
        check(!std::filesystem::exists(output), "Неполный результат неверного ключа");
        std::filesystem::remove_all(directory);
        std::cout << "Обратные числа: " << inverse_cases << " проверок.\n"
                  << "Малые обмены: " << exchanges << " проверок.\n"
                  << "Файловые циклы: " << file_cases << ", повреждённые форматы: " << malformed.size() << ".\n"
                  << "Все проверки лабораторной №4 пройдены.\n";
    } catch (const std::exception& error) {
        std::cerr << "Ошибка теста: " << error.what() << '\n';
        return 1;
    }
}
