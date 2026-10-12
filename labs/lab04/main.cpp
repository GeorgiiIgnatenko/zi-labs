#include "files.hpp"

#include <iostream>
#include <sstream>

std::string read_line(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
        throw std::runtime_error("Ввод завершён.");
    }
    return line;
}

long long read_number(const std::string& prompt, long long low, long long high) {
    while (true) {
        std::istringstream input(read_line(prompt));
        long long value;
        char extra;
        if ((input >> value) && !(input >> extra) && value >= low && value <= high) {
            return value;
        }
        std::cout << "Введите целое число от " << low << " до " << high << ".\n";
    }
}

void print_parameters(const crypto::ShamirParameters& key) {
    std::cout << "p = " << key.p << "\nСекреты (показаны для изучения):\n"
              << "C_A = " << key.c_a << ", D_A = " << key.d_a << '\n'
              << "C_B = " << key.c_b << ", D_B = " << key.d_b << '\n'
              << "Проверка: C_A*D_A mod (p-1) = " << key.c_a * key.d_a % (key.p - 1)
              << ", C_B*D_B mod (p-1) = " << key.c_b * key.d_b % (key.p - 1) << '\n';
}

int main() {
    std::cout << "Лабораторная работа №4. Шифр Шамира\n"
                 "Пути вводите целиком, без кавычек. Для результата нужно новое имя.\n";
    try {
        while (true) {
            std::cout << "\n1. Зашифровать файл: ввести p, C_A, C_B\n"
                         "2. Зашифровать файл: сгенерировать параметры\n"
                         "3. Расшифровать файл с сохранённым D_B\n"
                         "4. Показать числовой пример из лекции\n"
                         "0. Выход\n";
            const auto choice = read_number("Выбор: ", 0, 4);
            if (choice == 0) {
                return 0;
            }
            try {
                if (choice == 4) {
                    const auto key = crypto::make_shamir_parameters(23, 7, 5);
                    const auto steps = crypto::shamir_exchange(10, key);
                    print_parameters(key);
                    std::cout << "m = 10 -> x1 = " << steps.x1 << " -> x2 = " << steps.x2
                              << " -> x3 = " << steps.x3 << " -> x4 = " << steps.x4 << '\n';
                } else if (choice == 3) {
                    const auto source = read_line("Зашифрованный файл: ");
                    const auto target = read_line("Восстановить в новый файл: ");
                    const auto d_b = read_number("D_B (сохранённый при шифровании): ", 3, crypto::max_number - 2);
                    const auto count = lab04::decrypt_file(source, target, d_b);
                    std::cout << "Расшифровано байтов: " << count << ". Результат: " << target << '\n';
                } else {
                    crypto::ShamirParameters key;
                    if (choice == 1) {
                        const auto p = read_number("p (простое, больше 255): ", 257, crypto::max_number);
                        const auto c_a = read_number("C_A: ", 3, p - 2);
                        const auto c_b = read_number("C_B: ", 3, p - 2);
                        key = crypto::make_shamir_parameters(p, c_a, c_b);
                    } else {
                        key = crypto::generate_shamir_parameters();
                    }
                    print_parameters(key);
                    std::cout << "Сохраните D_B = " << key.d_b << " для последующего расшифрования.\n";
                    const auto source = read_line("Исходный файл: ");
                    const auto target = read_line("Зашифровать в новый файл: ");
                    const auto count = lab04::encrypt_file(source, target, key);
                    std::cout << "Зашифровано байтов: " << count << ". Результат: " << target << '\n';
                }
            } catch (const std::exception& error) {
                if (std::cin.eof()) {
                    throw;
                }
                std::cout << "Ошибка: " << error.what() << '\n';
            }
        }
    } catch (const std::exception& error) {
        std::cout << '\n' << error.what() << '\n';
        return std::cin.eof() ? 0 : 1;
    }
}
