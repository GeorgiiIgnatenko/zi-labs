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

struct ShamirParameters {
    long long p;
    long long c_a;
    long long c_b;
    long long d_a;
    long long d_b;
};

struct ShamirSteps {
    long long x1;
    long long x2;
    long long x3;
    long long x4;
};

// Обратное a по модулю: a*d % modulus == 1.
// 2 <= modulus <= max_number, 1 <= a < modulus, gcd(a, modulus) == 1.
long long mod_inverse(long long a, long long modulus);

// Простое 5 <= p <= max_number; 2 < c_a,c_b < p-1, оба взаимно просты с p-1.
ShamirParameters make_shamir_parameters(long long p, long long c_a, long long c_b);
ShamirParameters generate_shamir_parameters();
void validate_shamir_parameters(const ShamirParameters& parameters);

// Все четыре шага для одного числа 0 <= message < p (пример из лекции).
ShamirSteps shamir_exchange(long long message, const ShamirParameters& parameters);

enum class SignatureAlgorithm { elgamal, gost94, dsa };

struct SignaturePublicKey {
    SignatureAlgorithm algorithm;
    long long p;
    long long q; // Для Эль-Гамаля q=0; для ГОСТ/DSA — простой порядок подгруппы.
    long long g; // В лекции для ГОСТ/DSA обозначается a.
    long long y;
};

struct SignatureKeyPair {
    SignaturePublicKey public_key;
    long long x;
};

struct Signature {
    long long r;
    long long s;
};

void validate_signature_public_key(const SignaturePublicKey& key);
void validate_signature_key_pair(const SignatureKeyPair& key);
SignatureKeyPair make_signature_key(SignatureAlgorithm algorithm, long long p,
                                    long long q, long long g, long long x);
SignatureKeyPair generate_signature_key(SignatureAlgorithm algorithm);

// Числовая демонстрация: Эль-Гамаль 1<h<p, ГОСТ 0<h<q, DSA 0<=h<q.
// k задаётся явно; nullopt означает r=0 или s=0 в ГОСТ/DSA: выбрать другое k.
std::optional<Signature> sign_hash(long long h, const SignatureKeyPair& key, long long k);
Signature sign_hash_random(long long h, const SignatureKeyPair& key);
bool verify_hash(long long h, const SignaturePublicKey& key, const Signature& signature);

} // namespace crypto
