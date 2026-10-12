#include "../../common/crypto.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

// Тот же способ ввода, что в первой лабораторной: одно число на строке.
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

void print_solution(long long a, long long y, long long p) {
    std::cout << "a = " << a << ", y = " << y << ", p = " << p << '\n';
    const auto x = crypto::discrete_log(a, y, p);
    if (!x.has_value()) {
        std::cout << "Решения нет: этот остаток не получается степенями a по модулю p.\n";
        return;
    }
    std::cout << "Наименьший неотрицательный x = " << *x
              << "\nПроверка: " << a << '^' << *x << " mod " << p
              << " = " << crypto::mod_pow(a, *x, p) << '\n';
}

int main() {
    std::cout << "Лабораторная работа №2. Шаг младенца, шаг великана\n"
                 "Решаем a^x mod p = y при НОД(a, p) = 1.\n";
    try {
        while (true) {
            std::cout << "\n1. Ввести параметры вручную\n"
                         "2. Сгенерировать пример с решением\n"
                         "0. Выход\n";
            const auto choice = read_number("Выбор: ", 0, 2);
            if (choice == 0) {
                return 0;
            }
            try {
                if (choice == 1) {
                    const auto p = read_number("p (модуль): ", 2, crypto::max_number);
                    const auto a = read_number("a (основание): ", 1, p - 1);
                    const auto y = read_number("y (остаток): ", 0, p - 1);
                    print_solution(a, y, p);
                } else {
                    const auto example = crypto::generate_discrete_log_example();
                    std::cout << "Модуль p прошёл тест Ферма; простота вероятная.\n"
                              << "Исходный показатель при генерации: " << example.source_exponent << '\n'
                              << "Найденный показатель может быть меньше исходного из-за повторения остатков.\n";
                    print_solution(example.a, example.y, example.p);
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
