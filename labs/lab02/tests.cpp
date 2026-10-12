#include "../../common/crypto.hpp"

#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <vector>

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

void check_known_power(long long a, long long source_x, long long p) {
    const auto y = crypto::mod_pow(a, source_x, p);
    const auto x = crypto::discrete_log(a, y, p);
    check(x.has_value(), "Потеряно заведомо существующее решение");
    check(*x >= 0 && *x < p && *x <= source_x, "Нарушена граница или минимальность x");
    check(crypto::mod_pow(a, *x, p) == y, "Не прошла обратная подстановка");
}

int main() {
    try {
        check(crypto::discrete_log(2, 9, 23) == 5, "Пример из лекции");
        check(crypto::discrete_log(4, 4, 17) == 1, "Повторение остатков: нужно наибольшее j");
        check(crypto::discrete_log(1, 1, 2) == 0, "Модуль 2, нулевой показатель");
        check(crypto::discrete_log(6, 6, 7) == 1, "Основание p-1");
        check(crypto::discrete_log(3, 3, 8) == 1, "Составной модуль");
        check(!crypto::discrete_log(4, 3, 7), "Недостижимый остаток");
        check(!crypto::discrete_log(3, 2, 8), "Недостижимый остаток при составном модуле");
        check(!crypto::discrete_log(2, 0, 7), "Нулевой остаток при взаимно простом основании");
        check(!crypto::discrete_log(1, 2, 7), "Основание 1");

        // Независимый эталон: последовательно перечисляем все степени до p-1.
        long long cases = 0;
        for (long long p = 2; p <= 100; ++p) {
            for (long long a = 1; a < p; ++a) {
                if (std::gcd(a, p) != 1) {
                    continue;
                }
                std::vector<long long> expected(static_cast<std::size_t>(p), -1);
                long long value = 1;
                for (long long x = 0; x < p; ++x) {
                    if (expected[value] == -1) {
                        expected[value] = x;
                    }
                    value = value * a % p;
                }
                for (long long y = 0; y < p; ++y) {
                    const auto actual = crypto::discrete_log(a, y, p);
                    check(actual.value_or(-1) == expected[y], "BSGS отличается от полного перебора");
                    ++cases;
                }
            }
        }
        std::cout << "Полный перебор малых входов: " << cases << " случаев.\n";

        // 5 имеет порядок p-1 при этом простом p; эталонный y вычислен независимо.
        check(crypto::discrete_log(5, 924'760'166, 1'000'000'007) == 987'654'321,
              "Большой пример с известным минимальным показателем");
        check(!crypto::discrete_log(4, 5, 1'000'000'007), "Большой пример без решения");
        for (long long p : {1'000'000'007LL, 1'999'999'973LL, 2'000'000'000LL}) {
            for (long long x : {0LL, 1LL, 44'721LL, 987'654'321LL}) {
                check_known_power(3, x, p);
            }
        }
        for (int i = 0; i < 10; ++i) {
            const auto example = crypto::generate_discrete_log_example();
            check(example.p >= 2 && example.p <= crypto::max_number, "Граница p при генерации");
            check(example.a >= 1 && example.a < example.p, "Граница a при генерации");
            check(std::gcd(example.a, example.p) == 1, "Генерация невзаимно простого основания");
            check(example.source_exponent >= 0 && example.source_exponent <= example.p - 2,
                  "Граница исходного показателя");
            check(example.y == crypto::mod_pow(example.a, example.source_exponent, example.p),
                  "Остаток при генерации");
            check_known_power(example.a, example.source_exponent, example.p);
        }

        expect_invalid([] { crypto::discrete_log(2, 4, 8); });
        expect_invalid([] { crypto::discrete_log(2, 1, 8); });
        expect_invalid([] { crypto::discrete_log(0, 0, 7); });
        expect_invalid([] { crypto::discrete_log(-2, 4, 7); });
        expect_invalid([] { crypto::discrete_log(7, 1, 7); });
        expect_invalid([] { crypto::discrete_log(2, -1, 7); });
        expect_invalid([] { crypto::discrete_log(2, 7, 7); });
        expect_invalid([] { crypto::discrete_log(1, 0, 1); });
        expect_invalid([] { crypto::discrete_log(1, 0, 0); });
        expect_invalid([] { crypto::discrete_log(1, 0, -7); });
        expect_invalid([] { crypto::discrete_log(1, 1, crypto::max_number + 1); });
        expect_invalid([] { crypto::discrete_log(std::numeric_limits<long long>::min(), 1, 7); });
        expect_invalid([] { crypto::discrete_log(2, std::numeric_limits<long long>::max(), 7); });
        expect_invalid([] { crypto::discrete_log(2, 1, std::numeric_limits<long long>::max()); });
        std::cout << "Все проверки лабораторной №2 пройдены.\n";
    } catch (const std::exception& error) {
        std::cerr << "Ошибка проверки: " << error.what() << '\n';
        return 1;
    }
}
