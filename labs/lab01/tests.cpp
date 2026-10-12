#include "../../common/crypto.hpp"

#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>

void check(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void expect_invalid(const std::function<void()>& action) {
    try {
        action();
    } catch (const std::invalid_argument&) {
        return;
    }
    throw std::runtime_error("Некорректные параметры должны отклоняться");
}

void check_gcd(long long a, long long b) {
    const auto result = crypto::extended_gcd(a, b);
    check(result.gcd == std::gcd(a, b), "НОД отличается от std::gcd");
    check(a * result.x + b * result.y == result.gcd, "Нарушено равенство Безу");
}

int main() {
    try {
        check(crypto::mod_pow(3, 100, 7) == 4, "Пример из лекции");
        check(crypto::mod_pow(-2, 5, 13) == 7, "Отрицательное основание");
        check(crypto::mod_pow(0, 0, 7) == 1, "Соглашение для 0^0");
        check(crypto::mod_pow(7, 0, 1) == 0, "Модуль 1");
        check(crypto::mod_pow(0, 10, 17) == 0, "Нулевое основание");
        check(crypto::mod_pow(1'999'999'999, 2, 2'000'000'000) == 1, "Большое произведение");
        check(crypto::mod_pow(1'000'000'000, 1'000'000'006, 1'000'000'007) == 1,
              "Большой показатель");

        // Независимый медленный алгоритм для небольших показателей.
        for (long long a = -20; a <= 20; ++a) {
            for (long long p = 1; p <= 30; ++p) {
                long long expected = 1 % p;
                const long long base = (a % p + p) % p;
                for (long long x = 0; x <= 20; ++x) {
                    check(crypto::mod_pow(a, x, p) == expected, "Сравнение с обычным умножением");
                    expected = expected * base % p;
                }
            }
        }

        const long long edges[] = {-2'000'000'000, -28, -19, -1, 0, 1, 19, 28, 2'000'000'000};
        for (auto a : edges) {
            for (auto b : edges) {
                check_gcd(a, b);
            }
        }
        std::mt19937_64 generator(42);
        std::uniform_int_distribution<long long> values(-crypto::max_number, crypto::max_number);
        for (int i = 0; i < 10'000; ++i) {
            const auto a = values(generator);
            const auto b = values(generator);
            check_gcd(a, b);
        }

        for (long long n : {2LL, 3LL, 5LL, 97LL, 1'000'000'007LL, 1'999'999'973LL}) {
            check(crypto::fermat_prime(n), "Простое число отклонено");
        }
        // Для 9 все основания из [2, 7] обнаруживают составность.
        for (long long n : {-5LL, 0LL, 1LL, 4LL, 9LL, 100LL, 2'000'000'000LL}) {
            check(!crypto::fermat_prime(n), "Непростое число принято");
        }
        check(crypto::mod_pow(2, 560, 561) == 1, "Число Кармайкла проходит основание 2");
        check(crypto::extended_gcd(3, 561).gcd == 3, "Основание 3 обнаруживает делитель 561");

        for (bool primes : {false, true}) {
            for (int i = 0; i < 20; ++i) {
                const auto example = crypto::generate_gcd_example(primes);
                check(example.a >= (primes ? 2 : 1) && example.a <= crypto::max_number,
                      "Граница случайного a");
                check(example.b >= (primes ? 2 : 1) && example.b <= crypto::max_number,
                      "Граница случайного b");
                check(example.result.gcd == std::gcd(example.a, example.b), "Случайный НОД");
                check(example.a * example.result.x + example.b * example.result.y == example.result.gcd,
                      "Случайное равенство Безу");
            }
        }
        check(crypto::random_number(7, 7) == 7, "Диапазон из одного числа");
        expect_invalid([] { crypto::mod_pow(2, -1, 7); });
        expect_invalid([] { crypto::mod_pow(2, 3, 0); });
        expect_invalid([] { crypto::mod_pow(2, 3, -7); });
        expect_invalid([] { crypto::mod_pow(2, 3, crypto::max_number + 1); });
        expect_invalid([] { crypto::mod_pow(2, crypto::max_number + 1, 7); });
        expect_invalid([] { crypto::mod_pow(crypto::max_number + 1, 3, 7); });
        expect_invalid([] { crypto::extended_gcd(std::numeric_limits<long long>::min(), 1); });
        expect_invalid([] { crypto::extended_gcd(1, crypto::max_number + 1); });
        expect_invalid([] { crypto::fermat_prime(5, 0); });
        expect_invalid([] { crypto::fermat_prime(crypto::max_number + 1); });
        expect_invalid([] { crypto::random_number(10, 2); });
        std::cout << "Все проверки пройдены.\n";
    } catch (const std::exception& error) {
        std::cerr << "Ошибка проверки: " << error.what() << '\n';
        return 1;
    }
}
