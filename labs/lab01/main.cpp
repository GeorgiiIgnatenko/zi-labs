#include "../../common/crypto.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

// Одна строка — одно целое число. Ошибка ввода не ломает меню.
long long read_number(const std::string& prompt, long long low, long long high) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {
            throw std::runtime_error("Ввод завершён.");
        }
        std::istringstream input(line);
        long long value;
        char extra;
        if ((input >> value) && !(input >> extra) && value >= low && value <= high) {
            return value;
        }
        std::cout << "Введите целое число от " << low << " до " << high << ".\n";
    }
}

void print_gcd(long long a, long long b, const crypto::GcdResult& result) {
    std::cout << "a = " << a << ", b = " << b
              << "\nНОД = " << result.gcd
              << ", x = " << result.x << ", y = " << result.y
              << "\nПроверка: " << a << " * (" << result.x << ") + "
              << b << " * (" << result.y << ") = "
              << a * result.x + b * result.y << '\n';
}

int main() {
    std::cout << "Лабораторная работа №1. Основы криптографической библиотеки\n";
    const long long limit = crypto::max_number;

    try {
        while (true) {
            std::cout << "\n1. Быстрое возведение a^x mod p\n"
                         "2. Тест простоты Ферма\n"
                         "3. Расширенный алгоритм Евклида: ввод a, b\n"
                         "4. Алгоритм Евклида: случайные a, b\n"
                         "5. Алгоритм Евклида: вероятно простые a, b\n"
                         "0. Выход\n";
            const long long choice = read_number("Выбор: ", 0, 5);
            if (choice == 0) {
                return 0;
            }
            if (choice == 1) {
                const auto a = read_number("a: ", -limit, limit);
                const auto exponent = read_number("x (показатель): ", 0, limit);
                const auto modulus = read_number("p (модуль): ", 1, limit);
                std::cout << "Результат: " << crypto::mod_pow(a, exponent, modulus) << '\n';
            } else if (choice == 2) {
                const auto n = read_number("Проверяемое число: ", -limit, limit);
                if (crypto::fermat_prime(n)) {
                    std::cout << "Вероятно простое (тест Ферма, до 100 раундов).\n"
                                 "Тест не является доказательством простоты.\n";
                } else {
                    std::cout << (n < 2 ? "Не является простым.\n" : "Составное число.\n");
                }
            } else if (choice == 3) {
                const auto a = read_number("a: ", -limit, limit);
                const auto b = read_number("b: ", -limit, limit);
                print_gcd(a, b, crypto::extended_gcd(a, b));
            } else {
                const auto example = crypto::generate_gcd_example(choice == 5);
                if (choice == 5) {
                    std::cout << "Оба числа прошли тест Ферма (100 раундов каждое, кроме 2 и 3).\n";
                }
                print_gcd(example.a, example.b, example.result);
            }
        }
    } catch (const std::exception& error) {
        std::cout << '\n' << error.what() << '\n';
        return std::cin.eof() ? 0 : 1;
    }
}
