#pragma once

#include <optional>

namespace crypto {

// Произведение двух чисел этого диапазона помещается в long long.
inline constexpr long long max_number = 2'000'000'000;
inline constexpr int fermat_rounds = 100;

struct GcdResult {
    long long gcd;
    long long x;
    long long y;
};

struct GcdExample {
    long long a;
    long long b;
    GcdResult result;
};

struct DiscreteLogExample {
    long long a;
    long long y;
    long long p;
    long long source_exponent;
};

struct DiffieHellmanParameters {
    long long p;
    long long g;
    long long x_a;
    long long x_b;
};

struct DiffieHellmanResult {
    long long y_a;
    long long y_b;
    long long z_a;
    long long z_b;
};

// |a| <= max_number, 0 <= exponent <= max_number, 1 <= modulus <= max_number.
long long mod_pow(long long a, long long exponent, long long modulus);

// true означает «вероятно простое», а не доказанную простоту.
bool fermat_prime(long long n, int rounds = fermat_rounds);

// Возвращает gcd >= 0 и коэффициенты a*x + b*y = gcd, |a|, |b| <= max_number.
// Для (0, 0) принято gcd = 0, x = 1, y = 0.
GcdResult extended_gcd(long long a, long long b);

long long random_number(long long low, long long high);
long long generate_probable_prime();
GcdExample generate_gcd_example(bool prime_numbers);

// BSGS: минимальный x >= 0 для a^x mod p = y; nullopt, если решения нет.
// 2 <= p <= max_number, 1 <= a < p, 0 <= y < p, gcd(a, p) = 1.
std::optional<long long> discrete_log(long long a, long long y, long long p);

// Генерирует разрешимый пример; поиск x выполняется отдельно по a, y, p.
DiscreteLogExample generate_discrete_log_example();

// Точная проверка перебором делителей для |n| <= max_number.
// Отдельная функция: вероятностный тест Ферма первой лабы не меняется.
bool is_prime(long long n);

// Для простого p проверяет, что степени g порождают все ненулевые остатки.
// |g|, |p| <= max_number; непростой p или g вне [1, p-1] дают false.
bool is_primitive_root(long long g, long long p);

// Простое 5 <= p <= max_number, первообразный 2 <= g <= p-2,
// 2 <= x_a, x_b <= p-1. Оба общих ключа вычисляются независимо.
DiffieHellmanResult diffie_hellman(long long p, long long g, long long x_a, long long x_b);

// Выбирает простые p=2q+1 и q, подходящее g и оба секрета внутри функции.
DiffieHellmanParameters generate_diffie_hellman_parameters();

} // namespace crypto
