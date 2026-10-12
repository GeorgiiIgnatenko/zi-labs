#include "../../common/crypto.hpp"

#include <functional>
#include <iostream>
#include <limits>
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

long long slow_power(long long a, long long x, long long p) {
    long long result = 1;
    for (long long i = 0; i < x; ++i) {
        result = result * a % p;
    }
    return result;
}

// Независимая проверка корня для простого p: перечисляем все остатки.
bool brute_root(long long g, long long p) {
    std::vector<bool> seen(static_cast<std::size_t>(p), false);
    long long value = 1;
    long long count = 0;
    for (long long i = 0; i < p - 1; ++i) {
        if (!seen[value]) {
            seen[value] = true;
            ++count;
        }
        value = value * g % p;
    }
    return count == p - 1;
}

int main() {
    try {
        // Эталон для проверки простоты — решето, а не второй перебор делителей.
        std::vector<bool> prime(10'001, true);
        prime[0] = prime[1] = false;
        for (int d = 2; d * d <= 10'000; ++d) {
            if (prime[d]) {
                for (int multiple = d * d; multiple <= 10'000; multiple += d) {
                    prime[multiple] = false;
                }
            }
        }
        for (long long n = 0; n <= 10'000; ++n) {
            check(crypto::is_prime(n) == prime[n], "Тест простоты не совпал с решетом");
        }
        for (long long n : {-2'000'000'000LL, -1LL, 561LL, 1105LL, 1729LL, 2'000'000'000LL}) {
            check(!crypto::is_prime(n), "Непростое число принято");
        }
        check(crypto::is_prime(1'000'000'007), "Большое простое отклонено");
        check(crypto::is_prime(1'999'999'973), "Простое у верхней границы отклонено");

        long long root_cases = 0;
        for (long long p = 2; p <= 100; ++p) {
            for (long long g = 0; g <= p; ++g) {
                const bool expected = prime[p] && g >= 1 && g < p && brute_root(g, p);
                check(crypto::is_primitive_root(g, p) == expected, "Проверка порядка не совпала с перебором");
                ++root_cases;
            }
        }
        check(!crypto::is_primitive_root(3, 23), "Неподходящее g из лекции принято");
        check(crypto::is_primitive_root(10, 23), "Первообразный корень не обязан быть простым");

        const auto example = crypto::diffie_hellman(23, 5, 7, 13);
        check(example.y_a == 17 && example.y_b == 21, "Открытые значения примера лекции");
        check(example.z_a == 10 && example.z_b == 10, "Общий ключ примера лекции");
        const auto edge = crypto::diffie_hellman(23, 5, 22, 7);
        check(edge.y_a == 1 && edge.z_a == 1 && edge.z_b == 1, "Секрет p-1 разрешён лекцией");

        long long exchanges = 0;
        for (long long p = 5; p <= 31; ++p) {
            if (!prime[p]) {
                continue;
            }
            for (long long g = 2; g <= p - 2; ++g) {
                if (!brute_root(g, p)) {
                    continue;
                }
                for (long long x_a = 2; x_a <= p - 1; ++x_a) {
                    for (long long x_b = 2; x_b <= p - 1; ++x_b) {
                        const auto result = crypto::diffie_hellman(p, g, x_a, x_b);
                        const auto expected = slow_power(g, x_a * x_b, p);
                        check(result.y_a == slow_power(g, x_a, p), "Открытое число A");
                        check(result.y_b == slow_power(g, x_b, p), "Открытое число B");
                        check(result.z_a == expected && result.z_b == expected, "Ключи отличаются от эталона");
                        ++exchanges;
                    }
                }
            }
        }

        const auto big = crypto::diffie_hellman(1'000'000'007, 5, 123'456'789, 987'654'321);
        check(big.y_a == 372'224'738 && big.y_b == 924'760'166, "Большие открытые значения");
        check(big.z_a == 622'185'569 && big.z_b == 622'185'569, "Большой общий ключ");
        const auto upper = crypto::diffie_hellman(1'999'999'973, 2, 1'999'999'971, 1'999'999'970);
        check(upper.y_a == 999'999'987 && upper.y_b == 1'499'999'980, "Верхняя граница открытых значений");
        check(upper.z_a == 4 && upper.z_b == 4, "Верхняя граница общего ключа");

        for (int i = 0; i < 8; ++i) {
            const auto parameters = crypto::generate_diffie_hellman_parameters();
            const auto [p, g, x_a, x_b] = parameters;
            check(p > 1'000'000'000 && p <= crypto::max_number, "Граница сгенерированного p");
            check(crypto::is_prime(p) && crypto::is_prime((p - 1) / 2), "Генерация простых p и q");
            check(g >= 2 && g <= p - 2 && crypto::is_primitive_root(g, p), "Сгенерированное g");
            check(x_a >= 2 && x_a <= p - 1 && x_b >= 2 && x_b <= p - 1, "Границы секретов");
            const auto result = crypto::diffie_hellman(p, g, x_a, x_b);
            check(result.z_a == result.z_b && result.z_a > 0 && result.z_a < p, "Сгенерированный обмен");
        }

        expect_invalid([] { crypto::diffie_hellman(21, 2, 7, 13); });
        expect_invalid([] { crypto::diffie_hellman(561, 5, 7, 13); });
        expect_invalid([] { crypto::diffie_hellman(23, 3, 7, 13); });
        expect_invalid([] { crypto::diffie_hellman(23, 1, 7, 13); });
        expect_invalid([] { crypto::diffie_hellman(23, 22, 7, 13); });
        expect_invalid([] { crypto::diffie_hellman(23, 5, 1, 13); });
        expect_invalid([] { crypto::diffie_hellman(23, 5, 7, 23); });
        expect_invalid([] { crypto::diffie_hellman(23, 5, -7, 13); });
        expect_invalid([] { crypto::diffie_hellman(3, 2, 2, 2); });
        expect_invalid([] { crypto::diffie_hellman(-5, 2, 2, 2); });
        expect_invalid([] { crypto::diffie_hellman(crypto::max_number + 1, 2, 2, 2); });
        expect_invalid([] { crypto::diffie_hellman(23, 5, std::numeric_limits<long long>::min(), 7); });
        expect_invalid([] { crypto::diffie_hellman(std::numeric_limits<long long>::max(), 5, 7, 13); });
        expect_invalid([] { crypto::is_prime(crypto::max_number + 1); });
        expect_invalid([] { crypto::is_prime(std::numeric_limits<long long>::min()); });
        expect_invalid([] { crypto::is_primitive_root(std::numeric_limits<long long>::max(), 23); });
        expect_invalid([] { crypto::is_primitive_root(5, crypto::max_number + 1); });

        std::cout << "Простота: 10001 сравнение с решетом.\n"
                  << "Первообразные корни: " << root_cases << " сравнений с перебором.\n"
                  << "Малые обмены: " << exchanges << " проверок.\n"
                  << "Все проверки лабораторной №3 пройдены.\n";
    } catch (const std::exception& error) {
        std::cerr << "Ошибка проверки: " << error.what() << '\n';
        return 1;
    }
}
