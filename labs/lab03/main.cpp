#include "../../common/crypto.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

// Одно число на строке, как в первых двух лабораторных.
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

void print_exchange(const crypto::DiffieHellmanParameters& parameters) {
    const auto [p, g, x_a, x_b] = parameters;
    const auto result = crypto::diffie_hellman(p, g, x_a, x_b);
    std::cout << "Общие открытые параметры: p = " << p << ", g = " << g << '\n'
              << "Секреты (показаны для изучения): X_A = " << x_a << ", X_B = " << x_b << '\n'
              << "A отправляет B: Y_A = " << g << '^' << x_a << " mod " << p
              << " = " << result.y_a << '\n'
              << "B отправляет A: Y_B = " << g << '^' << x_b << " mod " << p
              << " = " << result.y_b << '\n'
              << "A вычисляет: Z_A = " << result.y_b << '^' << x_a << " mod " << p
              << " = " << result.z_a << '\n'
              << "B вычисляет: Z_B = " << result.y_a << '^' << x_b << " mod " << p
              << " = " << result.z_b << '\n'
              << "Проверка: " << (result.z_a == result.z_b ? "ключи совпадают." : "ключи не совпадают.")
              << "\nОбщий ключ: " << result.z_a << '\n';
}

int main() {
    std::cout << "Лабораторная работа №3. Схема Диффи — Хеллмана\n";
    try {
        while (true) {
            std::cout << "\n1. Ввести p, g, X_A, X_B вручную\n"
                         "2. Сгенерировать параметры\n"
                         "3. Показать пример из лекции\n"
                         "0. Выход\n";
            const auto choice = read_number("Выбор: ", 0, 3);
            if (choice == 0) {
                return 0;
            }
            try {
                if (choice == 1) {
                    const auto p = read_number("p (простой модуль): ", 5, crypto::max_number);
                    const auto g = read_number("g (первообразный корень): ", 2, p - 2);
                    const auto x_a = read_number("X_A (секрет A): ", 2, p - 1);
                    const auto x_b = read_number("X_B (секрет B): ", 2, p - 1);
                    print_exchange({p, g, x_a, x_b});
                } else if (choice == 2) {
                    const auto parameters = crypto::generate_diffie_hellman_parameters();
                    std::cout << "p = 2*q + 1, q = " << (parameters.p - 1) / 2
                              << ". Простота p и q проверена точно.\n";
                    print_exchange(parameters);
                } else {
                    std::cout << "Лекция 3, страница 17: p=23, g=5, X_A=7, X_B=13.\n";
                    print_exchange({23, 5, 7, 13});
                }
            } catch (const std::invalid_argument& error) {
                std::cout << "Ошибка параметров: " << error.what() << '\n';
            }
        }
    } catch (const std::exception& error) {
        std::cout << '\n' << error.what() << '\n';
        return std::cin.eof() ? 0 : 1;
    }
}
